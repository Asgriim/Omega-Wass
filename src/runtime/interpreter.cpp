#include "runtime/interpreter.hpp"
#include "runtime/init.hpp"
#include <iostream>
#include "util/util.hpp"
#include <array>

using namespace omega::wass;

template <typename T, std::size_t... I>
i64 callNativeSeq(NativeFuncType fn,
                  std::vector<T>& args,
                  std::index_sequence<I...>)
{
    return fn(args[I]...);
}

template <typename T, std::size_t N>
i64 callNativeImp(NativeFuncType fn, std::vector<T>& args)
{
    return callNativeSeq<T>(fn, args, std::make_index_sequence<N>{});
}

static constexpr std::size_t MAX_NATIVE_ARGS = 10;

template <std::size_t... Ns>
constexpr auto make_callers(std::index_sequence<Ns...>) {
    using Fn = i64(*)(NativeFuncType, std::vector<void*>&);

    return std::array<Fn, sizeof...(Ns) + 1>{{
                                                     nullptr,
                                                     (+[](NativeFuncType fn, std::vector<void*>& args) {
                                                         return callNativeImp<void*, Ns + 1>(fn, args);
                                                     })...
                                             }};
}

static constexpr auto callers =
        make_callers(std::make_index_sequence<MAX_NATIVE_ARGS>{});

namespace omega::wass {

void Interpreter::init(module::WasmModule &module) {
    store_.init(module);
    u32 start_ind = findStartFuncInd(module);
    createFrame(start_ind);
}

void Interpreter::createFrame(u32 f_ind) {
    auto f_ptr = &store_.getFunc(f_ind);
    frame_stack_.push({});

    top_frame_ = &frame_stack_.top();
    top_frame_->func = f_ptr;
    top_frame_->code = f_ptr->code;
    top_frame_->labels = &f_ptr->labelMap;
    top_frame_->locals =  f_ptr->locals;
}

i64 Interpreter::readLEB128() {
    int64_t result = 0;
    unsigned shift = 0;
    uint8_t byte = 0;

    for (unsigned i = 0; i < util::MAX_LEB128_BYTES; i++) {
        byte = top_frame_->code[top_frame_->ip++];
        result |= int64_t(byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            // Last byte—perform sign extension if needed
            if ((byte & 0x40) && shift < 64) {
                result |= - (int64_t(1) << shift);
            }
            return result;
        }
    }
    throw std::overflow_error("SLEB128 exceeds maximum length for int64_t");
}

void Interpreter::start() {
    threadedCode();
}

void Interpreter::popFrame() {
    frame_stack_.pop();
    top_frame_ = &frame_stack_.top();
}


void Interpreter::callFunc(u32 f_ind) {
    RuntimeFunction &f = store_.getFunc(f_ind);
    if (f.isNative) {
        callNative(f);
    } else {
        createFrame(f_ind);
    }
}

void Interpreter::callNative(RuntimeFunction &f) {
    auto &sig = f.signature;
    size_t p_count = sig.params.size();
    std::vector<void*> args(p_count);

    if (p_count == 0 || p_count > MAX_NATIVE_ARGS)
        throw std::runtime_error("unsupported native arg count");

    auto caller = callers[p_count];
    if (!caller)
        throw std::runtime_error("no caller for this arg count");

    for (i64 i = p_count - 1; i >= 0; --i) {
        auto op = operand_stack_.top();
        operand_stack_.pop();

        if (sig.params[i] == REF) {
            args[i] = store_.getMem(0, op.val.i);
        } else {
            args[i] = reinterpret_cast<void *>(op.val.i);
        }
    }
    Operand ret(REF, 0L);
    ret.val.i = caller(f.native_ptr, args);

    if (!sig.results.empty()) {
        ret.type = sig.results[0];
        operand_stack_.push(ret);
    }
}


#define UNIMPLEMENTED(op)                                            \
    std::cerr << "Unimplemented opcode: " << (op)                    \
              << " at IP=" << top_frame_->ip - 1                     \
              << std::endl;                                          \
    std::abort()                                                     \

#define DISPATCH() goto *dispatch_table[top_frame_->code[top_frame_->ip++]]

void Interpreter::threadedCode() {
    i64 arg_int = 0;
    f64 arg_f   = 0;
    Operand op1, op2, op3;
    ControlBlock curr_block;
    Operand *cur_local;
//    runtime::Bytecode instr = static_cast<runtime::Bytecode>(top_frame_->code[top_frame_->ip++]);
    // Direct-threading dispatch table
    static void* dispatch_table[] = {
            [runtime::Bytecode::unreachable] = &&unreachable,
            [runtime::Bytecode::nop] = &&nop,
            [runtime::Bytecode::block] = &&block,
            [runtime::Bytecode::loop] = &&loop,
            [runtime::Bytecode::if_] = &&if_,
            [runtime::Bytecode::else_] = &&else_,
            [runtime::Bytecode::end] = &&end,
            [runtime::Bytecode::br] = &&br,
            [runtime::Bytecode::br_if] = &&br_if,
            [runtime::Bytecode::br_table] = &&br_table,
            [runtime::Bytecode::return_] = &&return_,
            [runtime::Bytecode::call] = &&call,
            [runtime::Bytecode::return_call] = &&return_call,
            [runtime::Bytecode::return_call_indirect] = &&return_call_indirect,
            [runtime::Bytecode::call_ref] = &&call_ref,
            [runtime::Bytecode::drop] = &&drop,
            [runtime::Bytecode::select] = &&select,
            [runtime::Bytecode::select_t] = &&select_t,
            [runtime::Bytecode::local_get] = &&local_get,
            [runtime::Bytecode::local_set] = &&local_set,
            [runtime::Bytecode::local_tee] = &&local_tee,
            [runtime::Bytecode::global_get] = &&global_get,
            [runtime::Bytecode::global_set] = &&global_set,
            [runtime::Bytecode::table_get] = &&table_get,
            [runtime::Bytecode::table_set] = &&table_set,
            [runtime::Bytecode::i32_load] = &&i32_load,
            [runtime::Bytecode::i64_load] = &&i64_load,
            [runtime::Bytecode::f32_load] = &&f32_load,
            [runtime::Bytecode::f64_load] = &&f64_load,
            [runtime::Bytecode::i32_load8_s] = &&i32_load8_s,
            [runtime::Bytecode::i32_load8_u] = &&i32_load8_u,
            [runtime::Bytecode::i32_load16_s] = &&i32_load16_s,
            [runtime::Bytecode::i32_load16_u] = &&i32_load16_u,
            [runtime::Bytecode::i64_load8_s] = &&i64_load8_s,
            [runtime::Bytecode::i64_load8_u] = &&i64_load8_u,
            [runtime::Bytecode::i64_load16_s] = &&i64_load16_s,
            [runtime::Bytecode::i64_load16_u] = &&i64_load16_u,
            [runtime::Bytecode::i64_load32_s] = &&i64_load32_s,
            [runtime::Bytecode::i64_load32_u] = &&i64_load32_u,
            [runtime::Bytecode::i32_store] = &&i32_store,
            [runtime::Bytecode::i64_store] = &&i64_store,
            [runtime::Bytecode::f32_store] = &&f32_store,
            [runtime::Bytecode::f64_store] = &&f64_store,
            [runtime::Bytecode::i32_store8] = &&i32_store8,
            [runtime::Bytecode::i32_store16] = &&i32_store16,
            [runtime::Bytecode::i64_store8] = &&i64_store8,
            [runtime::Bytecode::i64_store16] = &&i64_store16,
            [runtime::Bytecode::i64_store32] = &&i64_store32,
            [runtime::Bytecode::memory_size] = &&memory_size,
            [runtime::Bytecode::memory_grow] = &&memory_grow,
            [runtime::Bytecode::i32_const] = &&i32_const,
            [runtime::Bytecode::i64_const] = &&i64_const,
            [runtime::Bytecode::f32_const] = &&f32_const,
            [runtime::Bytecode::f64_const] = &&f64_const,
            [runtime::Bytecode::i32_eqz] = &&i32_eqz,
            [runtime::Bytecode::i32_eq] = &&i32_eq,
            [runtime::Bytecode::i32_ne] = &&i32_ne,
            [runtime::Bytecode::i32_lt_s] = &&i32_lt_s,
            [runtime::Bytecode::i32_lt_u] = &&i32_lt_u,
            [runtime::Bytecode::i32_gt_s] = &&i32_gt_s,
            [runtime::Bytecode::i32_gt_u] = &&i32_gt_u,
            [runtime::Bytecode::i32_le_s] = &&i32_le_s,
            [runtime::Bytecode::i32_le_u] = &&i32_le_u,
            [runtime::Bytecode::i32_ge_s] = &&i32_ge_s,
            [runtime::Bytecode::i32_ge_u] = &&i32_ge_u,
            [runtime::Bytecode::i64_eqz] = &&i64_eqz,
            [runtime::Bytecode::i64_eq] = &&i64_eq,
            [runtime::Bytecode::i64_ne] = &&i64_ne,
            [runtime::Bytecode::i64_lt_s] = &&i64_lt_s,
            [runtime::Bytecode::i64_lt_u] = &&i64_lt_u,
            [runtime::Bytecode::i64_gt_s] = &&i64_gt_s,
            [runtime::Bytecode::i64_gt_u] = &&i64_gt_u,
            [runtime::Bytecode::i64_le_s] = &&i64_le_s,
            [runtime::Bytecode::i64_le_u] = &&i64_le_u,
            [runtime::Bytecode::i64_ge_s] = &&i64_ge_s,
            [runtime::Bytecode::i64_ge_u] = &&i64_ge_u,
            [runtime::Bytecode::f32_eq] = &&f32_eq,
            [runtime::Bytecode::f32_ne] = &&f32_ne,
            [runtime::Bytecode::f32_lt] = &&f32_lt,
            [runtime::Bytecode::f32_gt] = &&f32_gt,
            [runtime::Bytecode::f32_le] = &&f32_le,
            [runtime::Bytecode::f32_ge] = &&f32_ge,
            [runtime::Bytecode::f64_eq] = &&f64_eq,
            [runtime::Bytecode::f64_ne] = &&f64_ne,
            [runtime::Bytecode::f64_lt] = &&f64_lt,
            [runtime::Bytecode::f64_gt] = &&f64_gt,
            [runtime::Bytecode::f64_le] = &&f64_le,
            [runtime::Bytecode::f64_ge] = &&f64_ge,
            [runtime::Bytecode::i32_clz] = &&i32_clz,
            [runtime::Bytecode::i32_ctz] = &&i32_ctz,
            [runtime::Bytecode::i32_popcnt] = &&i32_popcnt,
            [runtime::Bytecode::i32_add] = &&i32_add,
            [runtime::Bytecode::i32_sub] = &&i32_sub,
            [runtime::Bytecode::i32_mul] = &&i32_mul,
            [runtime::Bytecode::i32_div_s] = &&i32_div_s,
            [runtime::Bytecode::i32_div_u] = &&i32_div_u,
            [runtime::Bytecode::i32_rem_s] = &&i32_rem_s,
            [runtime::Bytecode::i32_rem_u] = &&i32_rem_u,
            [runtime::Bytecode::i32_and] = &&i32_and,
            [runtime::Bytecode::i32_or] = &&i32_or,
            [runtime::Bytecode::i32_xor] = &&i32_xor,
            [runtime::Bytecode::i32_shl] = &&i32_shl,
            [runtime::Bytecode::i32_shr_s] = &&i32_shr_s,
            [runtime::Bytecode::i32_shr_u] = &&i32_shr_u,
            [runtime::Bytecode::i32_rotl] = &&i32_rotl,
            [runtime::Bytecode::i32_rotr] = &&i32_rotr,
            [runtime::Bytecode::i64_clz] = &&i64_clz,
            [runtime::Bytecode::i64_ctz] = &&i64_ctz,
            [runtime::Bytecode::i64_popcnt] = &&i64_popcnt,
            [runtime::Bytecode::i64_add] = &&i64_add,
            [runtime::Bytecode::i64_sub] = &&i64_sub,
            [runtime::Bytecode::i64_mul] = &&i64_mul,
            [runtime::Bytecode::i64_div_s] = &&i64_div_s,
            [runtime::Bytecode::i64_div_u] = &&i64_div_u,
            [runtime::Bytecode::i64_rem_s] = &&i64_rem_s,
            [runtime::Bytecode::i64_rem_u] = &&i64_rem_u,
            [runtime::Bytecode::i64_and] = &&i64_and,
            [runtime::Bytecode::i64_or] = &&i64_or,
            [runtime::Bytecode::i64_xor] = &&i64_xor,
            [runtime::Bytecode::i64_shl] = &&i64_shl,
            [runtime::Bytecode::i64_shr_s] = &&i64_shr_s,
            [runtime::Bytecode::i64_shr_u] = &&i64_shr_u,
            [runtime::Bytecode::i64_rotl] = &&i64_rotl,
            [runtime::Bytecode::i64_rotr] = &&i64_rotr,
            [runtime::Bytecode::f32_abs] = &&f32_abs,
            [runtime::Bytecode::f32_neg] = &&f32_neg,
            [runtime::Bytecode::f32_ceil] = &&f32_ceil,
            [runtime::Bytecode::f32_floor] = &&f32_floor,
            [runtime::Bytecode::f32_trunc] = &&f32_trunc,
            [runtime::Bytecode::f32_nearest] = &&f32_nearest,
            [runtime::Bytecode::f32_sqrt] = &&f32_sqrt,
            [runtime::Bytecode::f32_add] = &&f32_add,
            [runtime::Bytecode::f32_sub] = &&f32_sub,
            [runtime::Bytecode::f32_mul] = &&f32_mul,
            [runtime::Bytecode::f32_div] = &&f32_div,
            [runtime::Bytecode::f32_min] = &&f32_min,
            [runtime::Bytecode::f32_max] = &&f32_max,
            [runtime::Bytecode::f32_copysign] = &&f32_copysign,
            [runtime::Bytecode::f64_abs] = &&f64_abs,
            [runtime::Bytecode::f64_neg] = &&f64_neg,
            [runtime::Bytecode::f64_ceil] = &&f64_ceil,
            [runtime::Bytecode::f64_floor] = &&f64_floor,
            [runtime::Bytecode::f64_trunc] = &&f64_trunc,
            [runtime::Bytecode::f64_nearest] = &&f64_nearest,
            [runtime::Bytecode::f64_sqrt] = &&f64_sqrt,
            [runtime::Bytecode::f64_add] = &&f64_add,
            [runtime::Bytecode::f64_sub] = &&f64_sub,
            [runtime::Bytecode::f64_mul] = &&f64_mul,
            [runtime::Bytecode::f64_div] = &&f64_div,
            [runtime::Bytecode::f64_min] = &&f64_min,
            [runtime::Bytecode::f64_max] = &&f64_max,
            [runtime::Bytecode::f64_copysign] = &&f64_copysign,
            [runtime::Bytecode::i32_wrap_i64] = &&i32_wrap_i64,
            [runtime::Bytecode::i32_trunc_s_f32] = &&i32_trunc_s_f32,
            [runtime::Bytecode::i32_trunc_u_f32] = &&i32_trunc_u_f32,
            [runtime::Bytecode::i32_trunc_s_f64] = &&i32_trunc_s_f64,
            [runtime::Bytecode::i32_trunc_u_f64] = &&i32_trunc_u_f64,
            [runtime::Bytecode::i64_extend_s_i32] = &&i64_extend_s_i32,
            [runtime::Bytecode::i64_extend_u_i32] = &&i64_extend_u_i32,
            [runtime::Bytecode::i64_trunc_s_f32] = &&i64_trunc_s_f32,
            [runtime::Bytecode::i64_trunc_u_f32] = &&i64_trunc_u_f32,
            [runtime::Bytecode::i64_trunc_s_f64] = &&i64_trunc_s_f64,
            [runtime::Bytecode::i64_trunc_u_f64] = &&i64_trunc_u_f64,
            [runtime::Bytecode::f32_convert_s_i32] = &&f32_convert_s_i32,
            [runtime::Bytecode::f32_convert_u_i32] = &&f32_convert_u_i32,
            [runtime::Bytecode::f32_convert_s_i64] = &&f32_convert_s_i64,
            [runtime::Bytecode::f32_convert_u_i64] = &&f32_convert_u_i64,
            [runtime::Bytecode::f32_demote_f64] = &&f32_demote_f64,
            [runtime::Bytecode::f64_convert_s_i32] = &&f64_convert_s_i32,
            [runtime::Bytecode::f64_convert_u_i32] = &&f64_convert_u_i32,
            [runtime::Bytecode::f64_convert_s_i64] = &&f64_convert_s_i64,
            [runtime::Bytecode::f64_convert_u_i64] = &&f64_convert_u_i64,
            [runtime::Bytecode::f64_promote_f32] = &&f64_promote_f32,
            [runtime::Bytecode::i32_reinterpret_f32] = &&i32_reinterpret_f32,
            [runtime::Bytecode::i64_reinterpret_f64] = &&i64_reinterpret_f64,
            [runtime::Bytecode::f32_reinterpret_i32] = &&f32_reinterpret_i32,
            [runtime::Bytecode::f64_reinterpret_i64] = &&f64_reinterpret_i64,
    };

    DISPATCH();

unreachable:
    UNIMPLEMENTED("unreachable");

nop:
    DISPATCH();

block:
    top_frame_->pushLabel();
    ++top_frame_->ip;
    DISPATCH();

loop:
    top_frame_->pushLabel();
    ++top_frame_->ip;
    DISPATCH();

if_:
    UNIMPLEMENTED("if_");

else_:
    UNIMPLEMENTED("else_");

end:
    if (top_frame_->control_stack.empty()) {
        if (frame_stack_.size() == 1) {
            return;
        }
        popFrame();
    } else {
        top_frame_->control_stack.pop_back();
    }
    DISPATCH();

br:
    arg_int = readLEB128();
    top_frame_->popBlocks(arg_int);

    curr_block = top_frame_->control_stack.back();

    if (curr_block.type == runtime::loop) {
        top_frame_->ip = curr_block.start;
    } else {
        top_frame_->ip = curr_block.end;
    }
    DISPATCH();

br_if:
    op1 = operand_stack_.top();
    operand_stack_.pop();

    if (op1.val.i) {
        goto br;
    }
    arg_int = readLEB128();
    DISPATCH();

br_table:
    UNIMPLEMENTED("br_table");

return_:
    top_frame_->ret();
    DISPATCH();

call:
    arg_int = readLEB128();
    callFunc(arg_int);
    DISPATCH();

return_call:
    UNIMPLEMENTED("return_call");

return_call_indirect:
    UNIMPLEMENTED("return_call_indirect");

call_ref:
    UNIMPLEMENTED("call_ref");

drop:
    operand_stack_.pop();
    DISPATCH();

select:
    op3 = operand_stack_.top();
    operand_stack_.pop();
    op2 = operand_stack_.top();
    operand_stack_.pop();
    op1 = operand_stack_.top();
    operand_stack_.pop();
    if (op3.val.i) {
        operand_stack_.push(op1);
    } else {
        operand_stack_.push(op2);
    }
    DISPATCH();

select_t:
    UNIMPLEMENTED("select_t");

local_get:
    arg_int = readLEB128();
    operand_stack_.push(top_frame_->locals[arg_int]);
    DISPATCH();

local_set:
    arg_int = readLEB128();
    op1 = operand_stack_.top();
    operand_stack_.pop();
    cur_local = &top_frame_->locals[arg_int];
    if (cur_local->type > ValType::F64) {
        cur_local->val.i = op1.val.i;
    } else {
        UNIMPLEMENTED("local_set with float type");
    }
    DISPATCH();

local_tee:
    UNIMPLEMENTED("local_tee");

global_get:
    UNIMPLEMENTED("global_get");

global_set:
    UNIMPLEMENTED("global_set");

table_get:
    UNIMPLEMENTED("table_get");

table_set:
    UNIMPLEMENTED("table_set");

i32_load:
    UNIMPLEMENTED("i32_load");

i64_load:
    UNIMPLEMENTED("i64_load");

f32_load:
    UNIMPLEMENTED("f32_load");

f64_load:
    UNIMPLEMENTED("f64_load");

i32_load8_s:
    UNIMPLEMENTED("i32_load8_s");

i32_load8_u:
    UNIMPLEMENTED("i32_load8_u");

i32_load16_s:
    UNIMPLEMENTED("i32_load16_s");

i32_load16_u:
    UNIMPLEMENTED("i32_load16_u");

i64_load8_s:
    UNIMPLEMENTED("i64_load8_s");

i64_load8_u:
    UNIMPLEMENTED("i64_load8_u");

i64_load16_s:
    UNIMPLEMENTED("i64_load16_s");

i64_load16_u:
    UNIMPLEMENTED("i64_load16_u");

i64_load32_s:
    UNIMPLEMENTED("i64_load32_s");

i64_load32_u:
    UNIMPLEMENTED("i64_load32_u");

i32_store:
    UNIMPLEMENTED("i32_store");

i64_store:
    UNIMPLEMENTED("i64_store");

f32_store:
    UNIMPLEMENTED("f32_store");

f64_store:
    UNIMPLEMENTED("f64_store");

i32_store8:
    UNIMPLEMENTED("i32_store8");

i32_store16:
    UNIMPLEMENTED("i32_store16");

i64_store8:
    UNIMPLEMENTED("i64_store8");

i64_store16:
    UNIMPLEMENTED("i64_store16");

i64_store32:
    UNIMPLEMENTED("i64_store32");

memory_size:
    UNIMPLEMENTED("memory_size");

memory_grow:
    UNIMPLEMENTED("memory_grow");

i32_const:
    arg_int = readLEB128();
    operand_stack_.emplace(I32, arg_int);
    DISPATCH();

i64_const:
    UNIMPLEMENTED("i64_const");

f32_const:
    UNIMPLEMENTED("f32_const");

f64_const:
    UNIMPLEMENTED("f64_const");

i32_eqz:
    UNIMPLEMENTED("i32_eqz");

i32_eq:
    UNIMPLEMENTED("i32_eq");

i32_ne:
    op1 = operand_stack_.top();
    operand_stack_.pop();
    op2 = operand_stack_.top();
    operand_stack_.pop();
    arg_int = op1.val.i != op2.val.i;
    operand_stack_.emplace(I32, arg_int);
    DISPATCH();

i32_lt_s:
    UNIMPLEMENTED("i32_lt_s");

i32_lt_u:
    UNIMPLEMENTED("i32_lt_u");

i32_gt_s:
    UNIMPLEMENTED("i32_gt_s");

i32_gt_u:
    UNIMPLEMENTED("i32_gt_u");

i32_le_s:
    UNIMPLEMENTED("i32_le_s");

i32_le_u:
    UNIMPLEMENTED("i32_le_u");

i32_ge_s:
    UNIMPLEMENTED("i32_ge_s");

i32_ge_u:
    UNIMPLEMENTED("i32_ge_u");

i64_eqz:
    UNIMPLEMENTED("i64_eqz");

i64_eq:
    UNIMPLEMENTED("i64_eq");

i64_ne:
    UNIMPLEMENTED("i64_ne");

i64_lt_s:
    UNIMPLEMENTED("i64_lt_s");

i64_lt_u:
    UNIMPLEMENTED("i64_lt_u");

i64_gt_s:
    UNIMPLEMENTED("i64_gt_s");

i64_gt_u:
    UNIMPLEMENTED("i64_gt_u");

i64_le_s:
    UNIMPLEMENTED("i64_le_s");

i64_le_u:
    UNIMPLEMENTED("i64_le_u");

i64_ge_s:
    UNIMPLEMENTED("i64_ge_s");

i64_ge_u:
    UNIMPLEMENTED("i64_ge_u");

f32_eq:
    UNIMPLEMENTED("f32_eq");

f32_ne:
    UNIMPLEMENTED("f32_ne");

f32_lt:
    UNIMPLEMENTED("f32_lt");

f32_gt:
    UNIMPLEMENTED("f32_gt");

f32_le:
    UNIMPLEMENTED("f32_le");

f32_ge:
    UNIMPLEMENTED("f32_ge");

f64_eq:
    UNIMPLEMENTED("f64_eq");

f64_ne:
    UNIMPLEMENTED("f64_ne");

f64_lt:
    UNIMPLEMENTED("f64_lt");

f64_gt:
    UNIMPLEMENTED("f64_gt");

f64_le:
    UNIMPLEMENTED("f64_le");

f64_ge:
    UNIMPLEMENTED("f64_ge");

i32_clz:
    UNIMPLEMENTED("i32_clz");

i32_ctz:
    UNIMPLEMENTED("i32_ctz");

i32_popcnt:
    UNIMPLEMENTED("i32_popcnt");

i32_add:
    op1 = operand_stack_.top();
    operand_stack_.pop();
    op2 = operand_stack_.top();
    operand_stack_.pop();
    arg_int = (op1.val.i + op2.val.i) & 0xFFFFFFFFu;
    operand_stack_.emplace(I32, arg_int);
    DISPATCH();

i32_sub:
    UNIMPLEMENTED("i32_sub");

i32_mul:
    UNIMPLEMENTED("i32_mul");

i32_div_s:
    UNIMPLEMENTED("i32_div_s");

i32_div_u:
    UNIMPLEMENTED("i32_div_u");

i32_rem_s:
    UNIMPLEMENTED("i32_rem_s");

i32_rem_u:
    UNIMPLEMENTED("i32_rem_u");

i32_and:
    op1 = operand_stack_.top();
    operand_stack_.pop();
    op2 = operand_stack_.top();
    operand_stack_.pop();
    arg_int = op1.val.i & op2.val.i;
    operand_stack_.emplace(I32, arg_int);
    DISPATCH();

i32_or:
    UNIMPLEMENTED("i32_or");

i32_xor:
    UNIMPLEMENTED("i32_xor");

i32_shl:
    UNIMPLEMENTED("i32_shl");

i32_shr_s:
    UNIMPLEMENTED("i32_shr_s");

i32_shr_u:
    UNIMPLEMENTED("i32_shr_u");

i32_rotl:
    UNIMPLEMENTED("i32_rotl");

i32_rotr:
    UNIMPLEMENTED("i32_rotr");

i64_clz:
    UNIMPLEMENTED("i64_clz");

i64_ctz:
    UNIMPLEMENTED("i64_ctz");

i64_popcnt:
    UNIMPLEMENTED("i64_popcnt");

i64_add:
    UNIMPLEMENTED("i64_add");

i64_sub:
    UNIMPLEMENTED("i64_sub");

i64_mul:
    UNIMPLEMENTED("i64_mul");

i64_div_s:
    UNIMPLEMENTED("i64_div_s");

i64_div_u:
    UNIMPLEMENTED("i64_div_u");

i64_rem_s:
    UNIMPLEMENTED("i64_rem_s");

i64_rem_u:
    UNIMPLEMENTED("i64_rem_u");

i64_and:
    UNIMPLEMENTED("i64_and");

i64_or:
    UNIMPLEMENTED("i64_or");

i64_xor:
    UNIMPLEMENTED("i64_xor");

i64_shl:
    UNIMPLEMENTED("i64_shl");

i64_shr_s:
    UNIMPLEMENTED("i64_shr_s");

i64_shr_u:
    UNIMPLEMENTED("i64_shr_u");

i64_rotl:
    UNIMPLEMENTED("i64_rotl");

i64_rotr:
    UNIMPLEMENTED("i64_rotr");

f32_abs:
    UNIMPLEMENTED("f32_abs");

f32_neg:
    UNIMPLEMENTED("f32_neg");

f32_ceil:
    UNIMPLEMENTED("f32_ceil");

f32_floor:
    UNIMPLEMENTED("f32_floor");

f32_trunc:
    UNIMPLEMENTED("f32_trunc");

f32_nearest:
    UNIMPLEMENTED("f32_nearest");

f32_sqrt:
    UNIMPLEMENTED("f32_sqrt");

f32_add:
    UNIMPLEMENTED("f32_add");

f32_sub:
    UNIMPLEMENTED("f32_sub");

f32_mul:
    UNIMPLEMENTED("f32_mul");

f32_div:
    UNIMPLEMENTED("f32_div");

f32_min:
    UNIMPLEMENTED("f32_min");

f32_max:
    UNIMPLEMENTED("f32_max");

f32_copysign:
    UNIMPLEMENTED("f32_copysign");

f64_abs:
    UNIMPLEMENTED("f64_abs");

f64_neg:
    UNIMPLEMENTED("f64_neg");

f64_ceil:
    UNIMPLEMENTED("f64_ceil");

f64_floor:
    UNIMPLEMENTED("f64_floor");

f64_trunc:
    UNIMPLEMENTED("f64_trunc");

f64_nearest:
    UNIMPLEMENTED("f64_nearest");

f64_sqrt:
    UNIMPLEMENTED("f64_sqrt");

f64_add:
    UNIMPLEMENTED("f64_add");

f64_sub:
    UNIMPLEMENTED("f64_sub");

f64_mul:
    UNIMPLEMENTED("f64_mul");

f64_div:
    UNIMPLEMENTED("f64_div");

f64_min:
    UNIMPLEMENTED("f64_min");

f64_max:
    UNIMPLEMENTED("f64_max");

f64_copysign:
    UNIMPLEMENTED("f64_copysign");

i32_wrap_i64:
    UNIMPLEMENTED("i32_wrap_i64");

i32_trunc_s_f32:
    UNIMPLEMENTED("i32_trunc_s_f32");

i32_trunc_u_f32:
    UNIMPLEMENTED("i32_trunc_u_f32");

i32_trunc_s_f64:
    UNIMPLEMENTED("i32_trunc_s_f64");

i32_trunc_u_f64:
    UNIMPLEMENTED("i32_trunc_u_f64");

i64_extend_s_i32:
    UNIMPLEMENTED("i64_extend_s_i32");

i64_extend_u_i32:
    UNIMPLEMENTED("i64_extend_u_i32");

i64_trunc_s_f32:
    UNIMPLEMENTED("i64_trunc_s_f32");

i64_trunc_u_f32:
    UNIMPLEMENTED("i64_trunc_u_f32");

i64_trunc_s_f64:
    UNIMPLEMENTED("i64_trunc_s_f64");

i64_trunc_u_f64:
    UNIMPLEMENTED("i64_trunc_u_f64");

f32_convert_s_i32:
    UNIMPLEMENTED("f32_convert_s_i32");

f32_convert_u_i32:
    UNIMPLEMENTED("f32_convert_u_i32");

f32_convert_s_i64:
    UNIMPLEMENTED("f32_convert_s_i64");

f32_convert_u_i64:
    UNIMPLEMENTED("f32_convert_u_i64");

f32_demote_f64:
    UNIMPLEMENTED("f32_demote_f64");

f64_convert_s_i32:
    UNIMPLEMENTED("f64_convert_s_i32");

f64_convert_u_i32:
    UNIMPLEMENTED("f64_convert_u_i32");

f64_convert_s_i64:
    UNIMPLEMENTED("f64_convert_s_i64");

f64_convert_u_i64:
    UNIMPLEMENTED("f64_convert_u_i64");

f64_promote_f32:
    UNIMPLEMENTED("f64_promote_f32");

i32_reinterpret_f32:
    UNIMPLEMENTED("i32_reinterpret_f32");

i64_reinterpret_f64:
    UNIMPLEMENTED("i64_reinterpret_f64");

f32_reinterpret_i32:
    UNIMPLEMENTED("f32_reinterpret_i32");

f64_reinterpret_i64:
    UNIMPLEMENTED("f64_reinterpret_i64");
}



}