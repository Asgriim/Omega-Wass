#ifndef OWASM_VM_WASM_VM_HPP
#define OWASM_VM_WASM_VM_HPP
#include "runtime_structures.hpp"
#include <stack>

namespace omega::wass {


class WassVM {
public:
    void initFromFile(std::string_view path);
    void start();
    Frame createFrame(int32_t func_ind);

private:
    void execute();

    ExecCtx ctx_;
    WasmModule module_;

};
}
#endif //OWASM_VM_WASM_VM_HPP
