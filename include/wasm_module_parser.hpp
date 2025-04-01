#ifndef OWASM_VM_WASM_MODULE_PARSER_HPP
#define OWASM_VM_WASM_MODULE_PARSER_HPP

#include "wasm_types.hpp"

namespace omega::wass {

class ModuleParser {
public:
    static WasmModule parseFromFile(std::string_view path);

};

} //namespace omega::wass

#endif //OWASM_VM_WASM_MODULE_PARSER_HPP
