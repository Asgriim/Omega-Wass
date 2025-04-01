#include "bytecode.hpp"
#include <iostream>
#include <cstdlib> // для std::abort

namespace omega::wass {
#define UNIMPLEMENTED(op)                                     \
        std::cerr << "[UNIMPLEMENTED] " << #op << "\n";       \
        std::abort();                                         \
                                                              \
                                                              \
InstructionHandler dispatch_table[256] = {};



 void init_dispatch_table() {
    dispatch_table[UNREACHABLE] = unreachable;
    dispatch_table[NOP] = nop;
    dispatch_table[BLOCK] = block;
    dispatch_table[LOOP] = loop;
    dispatch_table[IF] = if_op;
    dispatch_table[ELSE] = else_op;
    dispatch_table[END] = end;
    dispatch_table[BR_IF] = br_if;
    dispatch_table[CALL] = call;
    dispatch_table[DROP] = drop;
    dispatch_table[LOCAL_GET] = local_get;
    dispatch_table[LOCAL_SET] = local_set;
    dispatch_table[I32_CONST] = i32_const;
    dispatch_table[I32_ADD] = i32_add;
    dispatch_table[I32_REM_U] = i32_rem_u;
    dispatch_table[I32_EQZ] = i32_eqz;
    dispatch_table[I32_LE_S] = i32_le_s;
}


uint32_t findBlockEnd(const std::vector<u8>& code, uint32_t ip) {
    int depth = 1;
    while (ip < code.size()) {
        uint8_t op = code[ip++];
        if (op == 0x02 || op == 0x03 || op == 0x04) depth++; // block, loop, if
        else if (op == 0x0b) {
            depth--;
            if (depth == 0) return ip;
        }
    }
    throw std::runtime_error("Unmatched block end");
}

uint32_t findElseOrEnd(const std::vector<u8>& code, uint32_t ip) {
    int depth = 1;
    while (ip < code.size()) {
        uint8_t op = code[ip++];
        if (op == 0x02 || op == 0x03 || op == 0x04) depth++;     // block, loop, if
        else if (op == 0x05 && depth == 1) return ip;            // else
        else if (op == 0x0b) {
            depth--;
            if (depth == 0) return ip;
        }
    }
    throw std::runtime_error("Unmatched if/else/end");
}


void unreachable(Frame &curr_frame, ExecCtx *ctx) {
    UNIMPLEMENTED(unreachable);
}

void nop(Frame &curr_frame, ExecCtx *ctx) {
    UNIMPLEMENTED(nop);
}

void block(Frame &curr_frame, ExecCtx *ctx) {
    curr_frame.ip++; // пропускаем blocktype (обычно 0x40)

    ControlBlock cb {
            .type = BlockType::Block,
            .start_ip = curr_frame.ip,
            .end_ip = findBlockEnd(*curr_frame.code, curr_frame.ip), // нужно реализовать
            .stack_height = ctx->operandStack.size()
    };

    ctx->blockStack.push(cb);
}

void loop(Frame &curr_frame, ExecCtx *ctx) {
    curr_frame.ip++; // пропускаем blocktype (0x40)

    ControlBlock cb {
            .type = BlockType::Loop,
            .start_ip = curr_frame.ip,
            .end_ip = findBlockEnd(*curr_frame.code, curr_frame.ip),
            .stack_height = ctx->operandStack.size()
    };

    ctx->blockStack.push(cb);
}

void if_op(Frame &curr_frame, ExecCtx *ctx) {
    curr_frame.ip++; // skip blocktype (0x40)

    Operand cond = ctx->operandStack.top(); ctx->operandStack.pop();
    if (cond.type != ValType::I32) std::abort();

    uint32_t else_ip = findElseOrEnd(*curr_frame.code, curr_frame.ip);
    if (cond.val.w_int == 0) {
        curr_frame.ip = else_ip;
    }

    // else будет просто продолжением до end
    ControlBlock cb {
            .type = BlockType::If,
            .start_ip = curr_frame.ip,
            .end_ip = findBlockEnd(*curr_frame.code, curr_frame.ip),
            .stack_height = ctx->operandStack.size()
    };

    ctx->blockStack.push(cb);
}

void else_op(Frame &curr_frame, ExecCtx *ctx) {
    if (!ctx->blockStack.empty()) {
        auto &block = ctx->blockStack.top();
        curr_frame.ip = block.end_ip;
    } else {
        std::cerr << "Unexpected else\n";
        std::abort();
    }
}

void end(Frame &curr_frame, ExecCtx *ctx) {
    if (!ctx->blockStack.empty()) {
        ctx->blockStack.pop();
    } else if (!ctx->frameStack.empty()) {
        curr_frame = ctx->frameStack.top();
        ctx->frameStack.pop();
    } else {
        std::cerr << "End of execution\n";
    }
}

void br_if(Frame &curr_frame, ExecCtx *ctx) {
    uint8_t label = curr_frame.code->at(curr_frame.ip++);
    Operand cond = ctx->operandStack.top(); ctx->operandStack.pop();

    if (cond.type != ValType::I32) std::abort();

    if (cond.val.w_int != 0) {
        ControlBlock target;
        std::stack<ControlBlock> temp;

        for (int i = 0; i <= label; ++i) {
            if (ctx->blockStack.empty()) std::abort();
            target = ctx->blockStack.top(); ctx->blockStack.pop();
            temp.push(target);
        }

        // вернём обратно, не трогая стек
        while (!temp.empty()) {
            ctx->blockStack.push(temp.top());
            temp.pop();
        }

        // br к loop: прыгнуть на start_ip, иначе — на end_ip
        if (target.type == BlockType::Loop) {
            curr_frame.ip = target.start_ip;
        } else {
            curr_frame.ip = target.end_ip;
        }
    }
}

void call(Frame &curr_frame, ExecCtx *ctx) {
    uint8_t func_index = curr_frame.code->at(curr_frame.ip++);
    auto &func = ctx->store.getFunc()[func_index];

    //todo hardcoded printf
    if (func.isNative) {
        auto arg1 = ctx->operandStack.top();
        ctx->operandStack.pop();
        auto heap_pth = ctx->store.getHeap();
        int64_t a = func.ctx.native_ptr((heap_pth + arg1.val.w_int));
        Operand result;
        result.type = I64;
        result.val.w_int = a;
        ctx->operandStack.push(result);
        return;
    }

    Frame new_frame;
    new_frame.func_inst = &func;
    new_frame.code = &func.ctx.code->code;
    new_frame.ip = 0;

    // Параметры — снимаем со стека и пишем в locals
    uint32_t param_count = func.signature->params.size();
    for (int i = param_count - 1; i >= 0; --i) {
        Operand op = ctx->operandStack.top(); ctx->operandStack.pop();
        new_frame.locals.insert(new_frame.locals.begin(), op);
    }

    ctx->frameStack.push(curr_frame);
    curr_frame = new_frame;
    UNIMPLEMENTED(call);
}

void drop(Frame &curr_frame, ExecCtx *ctx) {
    if (!ctx->operandStack.empty()) {
        ctx->operandStack.pop();
    } else {
        std::cerr << "[VM] Operand stack underflow on drop\n";
        std::abort();
    }
}

void local_get(Frame &curr_frame, ExecCtx *ctx) {
    uint8_t idx = curr_frame.code->at(curr_frame.ip++);
    Operand value = curr_frame.locals[idx];
    ctx->operandStack.push(value);
}

void local_set(Frame &curr_frame, ExecCtx *ctx) {
    uint8_t idx = curr_frame.code->at(curr_frame.ip++);
    Operand value = ctx->operandStack.top();
    ctx->operandStack.pop();
    curr_frame.locals[idx] = value;
}

void i32_const(Frame &curr_frame, ExecCtx *ctx) {
    uint8_t *ptr = curr_frame.code->data() + curr_frame.ip;
    uint32_t value = 0;
    uint32_t shift = 0;

    while (true) {
        uint8_t byte = *ptr++;
        curr_frame.ip++;
        value |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
    }

    Operand op;
    op.type = ValType::I32;
    op.val.w_int = value;
    ctx->operandStack.push(op);
}

void i32_add(Frame &curr_frame, ExecCtx *ctx) {
    Operand b = ctx->operandStack.top(); ctx->operandStack.pop();
    Operand a = ctx->operandStack.top(); ctx->operandStack.pop();

    if (a.type != ValType::I32 || b.type != ValType::I32) {
        std::cerr << "[VM] Type mismatch in i32.add\n";
        std::abort();
    }

    Operand result;
    result.type = ValType::I32;
    result.val.w_int = static_cast<i32>(a.val.w_int) + static_cast<i32>(b.val.w_int);
    ctx->operandStack.push(result);
}

void i32_rem_u(Frame &curr_frame, ExecCtx *ctx) {
    Operand b = ctx->operandStack.top(); ctx->operandStack.pop();
    Operand a = ctx->operandStack.top(); ctx->operandStack.pop();

    Operand result;
    result.type = ValType::I32;
    result.val.w_int = static_cast<u32>(a.val.w_int) % static_cast<u32>(b.val.w_int);
    ctx->operandStack.push(result);
}

void i32_eqz(Frame &curr_frame, ExecCtx *ctx) {
    Operand op = ctx->operandStack.top(); ctx->operandStack.pop();
    if (op.type != ValType::I32) std::abort();

    Operand result;
    result.type = ValType::I32;
    result.val.w_int = (op.val.w_int == 0) ? 1 : 0;
    ctx->operandStack.push(result);
}

void i32_le_s(Frame &curr_frame, ExecCtx *ctx) {
    Operand b = ctx->operandStack.top(); ctx->operandStack.pop();
    Operand a = ctx->operandStack.top(); ctx->operandStack.pop();

    Operand result;
    result.type = ValType::I32;
    result.val.w_int = (static_cast<i32>(a.val.w_int) <= static_cast<i32>(b.val.w_int)) ? 1 : 0;
    ctx->operandStack.push(result);
}


}