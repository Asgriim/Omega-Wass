#include <getopt.h>
#include <filesystem>
#include <iostream>
#include "runtime/vm.hpp"
#include <fcntl.h>    // для creat(), O_* флагов
#include <sys/stat.h> // для mode_t и прав доступа
int main(int argc, char **argv) {
    std::string_view path;
    int64_t opt = 0;
    while (opt != -1) {
        opt = getopt(argc, argv, "m:");
        switch (opt) {
            case 'm': {
                path = optarg;
                break;
            }
        }
    }
    if (!std::filesystem::exists(path)){
        std::cerr << "WASM module not found";
        return -1;
    }

    omega::wass::Vm vm;
    vm.loadModule(path);
    vm.start();
    return 0;
}
