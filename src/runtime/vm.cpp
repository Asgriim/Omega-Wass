#include "runtime/vm.hpp"
#include "util/module_parser.hpp"

namespace omega::wass {

void Vm::loadModule(std::string_view path) {
    ModuleParser parser(path);
    module_ = parser.parseFromFile();
    interpreter_.init(module_);
}

void Vm::start() {
    interpreter_.start();
}
}