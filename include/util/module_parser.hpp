#ifndef OWASM_VM_MODULE_PARSER_HPP
#define OWASM_VM_MODULE_PARSER_HPP

#include "data/module_struct.hpp"
#include "util/buf_reader.hpp"

namespace omega::wass {
class ModuleParser {

public:
    ModuleParser(std::string_view path);

    module::WasmModule parseFromFile();
private:

    module::Limits parseLimits();

    std::vector<u8> parseExpr();

    template<typename Section>
    std::vector<Section> parseSection();

    template<typename Section>
    Section parseOneSectionEntry();
    std::vector<module::CustomSection> parseCustomSection();
private:
    util::BufReader bufReader_;
};

} //namespace omega::wass

#endif //OWASM_VM_MODULE_PARSER_HPP
