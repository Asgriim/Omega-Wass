
#ifndef OWASM_VM_VM_HPP
#define OWASM_VM_VM_HPP

#include "interpreter.hpp"

namespace omega::wass {
    class Vm {
    public:
        void loadModule(std::string_view path);
        void start();
    private:
        module::WasmModule module_;
        Interpreter interpreter_;
    };
}
#endif //OWASM_VM_VM_HPP
