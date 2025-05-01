
#ifndef OWASM_VM_INIT_HPP
#define OWASM_VM_INIT_HPP
#include "runtime_structs.hpp"
#include <gnu/lib-names.h>

namespace omega::wass {

LabelMap createLabelMap(const std::vector<u8> &code);

u32 findStartFuncInd(module::WasmModule &module);

std::vector<RuntimeFunction> initRuntimeFunctions(module::WasmModule &module);

GlobalsContainer initGlobals(module::WasmModule &module);

MemsContainer initMemory(module::WasmModule &module);

void initData(module::WasmModule &module, MemsContainer &mems);

}
#endif //OWASM_VM_INIT_HPP
