#ifndef OWASM_VM_RUNTIME_STRUCTS_HPP
#define OWASM_VM_RUNTIME_STRUCTS_HPP

#include "data/module_struct.hpp"
#include "bytecode/bytecode.hpp"
#include <unordered_map>
#include <stack>
#include <stdexcept>
namespace omega::wass {
constexpr u32 WASM_PAGE_SIZE = 1024 * 64;

struct ControlBlock {
    runtime::Bytecode type;
    u32 start;
    u32 end;
};

typedef int64_t (*NativeFuncType)(...);
using LabelMap = std::unordered_map<u32, ControlBlock>;
using MemsContainer = std::vector<std::vector<char>>;

union WasmVal {
    i64 i;
    f64 f;
};

struct Operand {
    Operand() = default;
    Operand(ValType t, i64 i) : type(t), val(i) {};
    Operand(ValType t, f64 f) : type(t), val(f) {};

    ValType type;
    WasmVal val;
};

struct GlobalVar {
    Operand op;
    bool mut;
};

using GlobalsContainer = std::vector<GlobalVar>;

struct RuntimeFunction {
    bool isNative = false;
    module::FuncSignature signature;
    std::vector<u8> code;
    std::vector<Operand> locals;
    LabelMap labelMap;

    NativeFuncType native_ptr;
};

using FunctionsContainer = std::vector<RuntimeFunction>;

struct Frame {

    void pushLabel() {
        control_stack.push_back(labels->find(ip)->second);
    }

    void popBlocks(i64 n) {
        for (i64 i = 0; i < n; ++i) {
            control_stack.pop_back();
        }
    }

    void ret() {
        control_stack.clear();
        ip = code.size() - 1;
    }

    RuntimeFunction *func;
    std::vector<u8> code;
    LabelMap *labels;
    std::vector<Operand> locals;
    std::deque<ControlBlock> control_stack;
    u32 ip = 0;
};

}
#endif //OWASM_VM_RUNTIME_STRUCTS_HPP
