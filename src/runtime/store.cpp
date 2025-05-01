#include "runtime/store.hpp"
#include "runtime/init.hpp"
namespace omega::wass {

void Store::init(module::WasmModule &module) {
    funcs_   = initRuntimeFunctions(module);
    globals_ = initGlobals(module);
    mems_    = initMemory(module);
    initData(module, mems_);
}

RuntimeFunction& Store::getFunc(u32 f_ind) {
    return funcs_[f_ind];
}

char *Store::getMem(u32 mem_ind, u32 ind) {
    return mems_[mem_ind].data() + ind;
}
}