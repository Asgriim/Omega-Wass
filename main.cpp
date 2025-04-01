#include "bytecode.hpp"
#include "wasm_vm.hpp"

int main(int argc, char **argv) {
    omega::wass::init_dispatch_table();

    std::string_view path = argv[1];
    omega::wass::WassVM wasmVm;
    wasmVm.initFromFile(path);
    wasmVm.start();
    return 0;
}
