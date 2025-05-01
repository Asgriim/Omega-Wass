
#ifndef OWASM_VM_STORE_HPP
#define OWASM_VM_STORE_HPP
#include "runtime_structs.hpp"

namespace omega::wass {
class Store {
public:
    void init(module::WasmModule &module);
    RuntimeFunction& getFunc(u32 f_ind);
    char* getMem(u32 mem_ind, u32 ind);
private:
    GlobalsContainer globals_;
    MemsContainer mems_;
    FunctionsContainer funcs_;
};
}

#endif //OWASM_VM_STORE_HPP
