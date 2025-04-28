#ifndef OWASM_VM_RUNTIME_STRUCTS_HPP
#define OWASM_VM_RUNTIME_STRUCTS_HPP

#include "data/module_struct.hpp"
#include "bytecode/bytecode.hpp"
#include <unordered_map>
#include <stack>
#include <stdexcept>
namespace omega::wass {

typedef int64_t (*NativeFuncType)(...);


struct ControlBlock {
    Bytecode type;
    u32 start;
    u32 end;
};

struct Operand {
    ValType type;
    union {
        i64 i = 0;
        f64 f;
    } val;
};

struct RuntimeFunction {
    bool isNative = false;
    module::FuncSignature signature;

    module::FunctionBody code;
    NativeFuncType native_ptr;
};
using LabelMap = std::unordered_map<u32, ControlBlock>;

//TODO MOVE
LabelMap createLabelMap(const std::vector<u8> &code) {
    LabelMap map;
    std::stack<ControlBlock*> control_stack;

    u32 size = code.size();
    for (u32 i = 0; i < size; ++i) {
        u8 op = code[i];
        switch (op) {
            case Bytecode::block:
            case Bytecode::loop:
            case Bytecode::if_: {
                u8 block_type = code.at(i+1);
                if (
                        block_type != ValType::BLOCK &&
                        block_type != ValType::F32 &&
                        block_type != ValType::F64 &&
                        block_type != ValType::I32 &&
                        block_type != ValType::I32
                        ) {
                    throw std::runtime_error("ILL FORMED BLOCK STRUCTURE");
                }
                ControlBlock block{.type = static_cast<Bytecode>(op), .start = (i + 2), .end = 0};
                map.insert({i, block});
                control_stack.push(&map.find(i)->second);
                break;
            }
            case Bytecode::end:
            case Bytecode::else_:{
                if (control_stack.empty()) {
                    if (i != size - 1) {
                        throw std::runtime_error("ILL FORMED BLOCK STRUCTURE");
                    }
                } else {
                    auto block = control_stack.top();
                    block->end = i;
                    control_stack.pop();
                }
            }
        }
    }
    return map;
}
}
#endif //OWASM_VM_RUNTIME_STRUCTS_HPP
