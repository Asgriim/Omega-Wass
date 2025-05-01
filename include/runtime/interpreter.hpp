
#ifndef OWASM_VM_INTERPRETER_HPP
#define OWASM_VM_INTERPRETER_HPP

#include "runtime/store.hpp"

namespace omega::wass {
class Interpreter {
public:
    void init(module::WasmModule &module);
    void start();
private:
    void threadedCode();
    i64 readLEB128();
    void createFrame(u32 f_ind);
    void popFrame();
    void callFunc(u32 f_ind);
    void callNative(RuntimeFunction &f);

    std::stack<Frame> frame_stack_;
    std::stack<Operand> operand_stack_;
    Frame *top_frame_ = nullptr;

    Store store_;

};
}
#endif //OWASM_VM_INTERPRETER_HPP
