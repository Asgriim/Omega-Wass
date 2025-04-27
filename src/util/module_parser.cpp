#include "util/module_parser.hpp"
#include <cstring>
#include "util/util.hpp"
#include <fstream>

namespace omega::wass {
using namespace module;

ModuleParser::ModuleParser(std::string_view path) : bufReader_(std::ifstream(path.data(), std::ios::binary)) {

}

WasmModule ModuleParser::parseFromFile() {
    WasmModule module;
    while (!bufReader_.isEnd()) {
        auto sectionId = static_cast<SectionType>(bufReader_.readLeb128());

        switch (sectionId) {
            case SectionType::Custom:
                //todo custom section not working
                module.customSection = parseCustomSection();
                break;
            case SectionType::Type:
                module.typesSection = parseSection<FuncSignature>();
                break;
            case SectionType::Import:
                module.importSection = parseSection<Import>();
                break;
            case SectionType::Function:
                module.functionSection = parseSection<FuncIndex>();
                break;
            case SectionType::Table:
                module.tableSection = parseSection<TableType>();
                break;
            case SectionType::Memory:
                module.memorySection = parseSection<Limits>();
                break;
            case SectionType::Global:
                module.globalSection = parseSection<Global>();
                break;
            case SectionType::Export:
                module.exportSection = parseSection<Export>();
                break;
            case SectionType::Start:
                module.startSection.functionIndex = bufReader_.readLeb128();
                break;
            case SectionType::Element:
                module.elementSection = parseSection<Element>();
                break;
            case SectionType::Code:
                module.codeSection = parseSection<FunctionBody>();
                break;
            case SectionType::Data:
                module.dataSection = parseSection<DataSegment>();
                break;
            case SectionType::DataCount:
                module.dataCountSection.count = bufReader_.readLeb128();
                break;
            default: {
                uint32_t sectionSize = bufReader_.readLeb128();
                bufReader_.next(sectionSize);
                break;
            }
        }
    }
    return module;

}

std::vector<CustomSection> ModuleParser::parseCustomSection() {
    std::vector<CustomSection> section;
    u64 sectionSize = bufReader_.readULeb128();
    bufReader_.next(sectionSize);

    return section;
}

Limits ModuleParser::parseLimits() {
    u8 flag = bufReader_.read<u8>();
    Limits limits;
    limits.min = bufReader_.readLeb128();

    if (flag == lims::max_flag) {
        limits.max = bufReader_.readLeb128();
    } else {
        limits.max = 0;
    }

    return limits;
}

std::vector<u8> ModuleParser::parseExpr() {
    std::vector<u8> expr;
    u8 op = bufReader_.read<u8>();
    while (op != expr::end) {
        expr.emplace_back(op);
        op = bufReader_.read<u8>();
    }
    expr.emplace_back(op);
    return expr;
}



template<typename Section>
Section ModuleParser::parseOneSectionEntry(){
    throw std::runtime_error("unimplemented");
};

template<>
CustomSection ModuleParser::parseOneSectionEntry() {
    //tbd debug this
    i64 sectionSize = bufReader_.readLeb128();
    bufReader_.next(sectionSize);
    return {};
}

template<>
FuncSignature ModuleParser::parseOneSectionEntry() {
    if (bufReader_.read<u8>() != functype::func) {
        throw std::runtime_error("invalid type form");
    }
    i64 p_count = bufReader_.readLeb128();
    std::vector<ValType> params;
    params.reserve(p_count);

    for (i64 i = 0; i < p_count; ++i) {
        params.emplace_back(bufReader_.read<ValType>());
    }

    i64 r_count = bufReader_.readLeb128();
    std::vector<ValType> results;

    for (i64 i = 0; i < r_count; ++i) {
        results.emplace_back(bufReader_.read<ValType>());
    }
    return {params, results};
}

template<>
Import ModuleParser::parseOneSectionEntry() {
    Import imp;
    imp.module = bufReader_.readStr();
    imp.name   = bufReader_.readStr();
    imp.kind   = bufReader_.read<ImportKind>();
    switch (imp.kind) {
        case ImportKind::FUNC: {
            imp.typeIndex = bufReader_.readLeb128();
            break;
        }
        case ImportKind::TABLE: {
            imp.tableLimits = parseLimits();
            break;
        }
        case ImportKind::MEMORY: {
            imp.memLimits = parseLimits();
            break;
        }
        case ImportKind::GLOBAL:
            imp.globalType.valType  = bufReader_.read<ValType>();
            imp.globalType.mutable_ = bufReader_.read<u8>();
            break;
    }
    return imp;
}

template<>
 FuncIndex ModuleParser::parseOneSectionEntry() {
    i64 ind = bufReader_.readLeb128();
    return {static_cast<i32>(ind)};
}

template<>
Limits ModuleParser::parseOneSectionEntry() {
    return parseLimits();
}

template<>
Global ModuleParser::parseOneSectionEntry() {
    Global g {
        .valType  = bufReader_.read<ValType>(),
        .mutable_ = bufReader_.read<MutableType>(),
        .initExpr = parseExpr()
    };
    return g;
}

template<>
Export ModuleParser::parseOneSectionEntry() {
    Export e {
        .name = bufReader_.readStr(),
        .kind = bufReader_.read<ExportKind>(),
        .index = static_cast<u32>(bufReader_.readLeb128())
    };

    return e;
}


template<>
Element ModuleParser::parseOneSectionEntry() {
    Element e;
    e.tableIndex = bufReader_.readLeb128();
    e.offsetExpr = parseExpr();
    i64 f_count = bufReader_.readLeb128();
    for (i64 i = 0; i < f_count; ++i) {
        e.functionIndices.emplace_back(bufReader_.readLeb128());
    }
    return e;
}


template<>
FunctionBody ModuleParser::parseOneSectionEntry() {
    FunctionBody body;
    i64 func_size = bufReader_.readLeb128();
    i64 locals_count = bufReader_.readLeb128();
    u8 *start = bufReader_.get();
    for (i64 i = 0; i < locals_count; ++i) {
        body.locals.emplace_back(bufReader_.readLeb128(), bufReader_.read<ValType>());
    }
    func_size -= bufReader_.get() - start + 1;
    body.code.reserve(func_size);
    body.code.assign(bufReader_.get(), bufReader_.get() + func_size);
    bufReader_.next(func_size);
    return body;
}


template<>
DataSegment ModuleParser::parseOneSectionEntry() {
    DataSegment d;
    d.memIndex = bufReader_.readLeb128();
    d.offsetExpr = parseExpr();
    i64 sz = bufReader_.readLeb128();
    //todo memcpy?
    d.data.reserve(sz);
    for (i64 i = 0; i < sz; ++i) {
        d.data.emplace_back(bufReader_.read<u8>());
    }
    return d;
}

template<typename Section>
std::vector<Section> ModuleParser::parseSection() {
    std::vector<Section> section;
    i64 sectionSizeByte = bufReader_.readLeb128();
    i64 sectionSize = bufReader_.readLeb128();

    for (i32 i = 0; i < sectionSize; ++i) {
        section.emplace_back(parseOneSectionEntry<Section>());
    }
    return section;
}


} //namespace omega::wass