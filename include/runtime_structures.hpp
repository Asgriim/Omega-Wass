#ifndef OWASM_VM_RUNTIME_STRUCTURES_HPP
#define OWASM_VM_RUNTIME_STRUCTURES_HPP

#include "wasm_types.hpp"
#include "stack"

namespace omega::wass {

constexpr uint32_t WASM_PAGE_SIZE = 1024 * 64;
typedef int64_t (*NativeFuncType)(...);



struct Operand {
    ValType type;
    union {
        i64 w_int = 0;
        double w_float;
    } val;
};

struct RuntimeFunction {
    bool isNative = false;
    FuncType *signature;

    union {
        FunctionBody *code;
        NativeFuncType native_ptr;
    } ctx;
};

//todo implement full
class Store {
public:
    void init(WasmModule &module);
    std::vector<RuntimeFunction>& getFunc();
    char* getHeap();

private:
    void initFunc(WasmModule &module);
    void initHeap(WasmModule &module);
    void initDataSection(WasmModule &module);

    std::vector<RuntimeFunction> func_;
    //todo refactor
    std::vector<char> heap;
};

struct Frame {
    RuntimeFunction *func_inst;
    std::vector<u8> *code;
    std::vector<Operand> locals;
    uint32_t ip = 0;
};

enum class BlockType {
    Block,
    Loop,
    If,
};

struct ControlBlock {
    BlockType type;
    uint32_t start_ip;     // ip начала тела блока
    uint32_t end_ip;       // ip инструкции `end`
    size_t stack_height;   // размер operandStack при входе в блок
};

struct ExecCtx {
    std::stack<Operand> operandStack;
    std::stack<Frame> frameStack;
    std::stack<ControlBlock> blockStack;
    Store store;
};

}// namespace runtime


#endif //OWASM_VM_RUNTIME_STRUCTURES_HPP
