#include <fstream>
#include <cstdint>
#include <cstring>
#include "wasm_module_parser.hpp"



namespace omega::wass {
constexpr size_t MODULE_OFFSET = 8; // 4 bytes magic + 4 bytes version

struct ParserBuffer {
    size_t offset = 0;
    uint8_t* buf = nullptr;

    [[nodiscard]]
    uint8_t* get() const noexcept { return buf + offset; }

    template<typename T>
    T read() {
        T t;
        std::memcpy(&t, buf + offset, sizeof(T));
        offset += sizeof(T);
        return t;
    }

    uint8_t readByte() { return read<uint8_t>(); }

    //todo move to
    uint32_t readLEB() {
        uint32_t result = 0;
        uint8_t shift = 0;
        while (true) {
            uint8_t byte = readByte();
            result |= (byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }
        return result;
    }

    std::string readName() {
        uint32_t len = readLEB();
        std::string str(reinterpret_cast<char*>(buf + offset), len);
        offset += len;
        return str;
    }

    ValType readValType() { return static_cast<ValType>(read<int8_t>()); }

    Limits readLimits() {
        uint8_t flags = readByte();
        Limits lim{.min = readLEB()};
        if (flags & 0x01) lim.max = readLEB();
        return lim;
    }
};

TypeSection parseTypeSection(ParserBuffer& b) {
    TypeSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        if (b.readByte() != 0x60) throw std::runtime_error("invalid type form");
        uint32_t pcount = b.readLEB();
        std::vector<ValType> params(pcount);
        for (auto& p : params)
            p = b.readValType();

        uint32_t rcount = b.readLEB();
        std::vector<ValType> results(rcount);
        for (auto& r : results)
            r = b.readValType();
        s.types.push_back({params, results});
    }
    b.offset = start + size;
    return s;
}

ImportSection parseImportSection(ParserBuffer& b) {
    ImportSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        Import imp;
        imp.module = b.readName();
        imp.name = b.readName();
        imp.kind = static_cast<ImportKind>(b.readByte());
        switch (imp.kind) {
            case FUNC: imp.typeIndex = b.readLEB(); break;
            case TABLE: imp.tableLimits = b.readLimits(); break;
            case MEMORY: imp.memLimits = b.readLimits(); break;
            case GLOBAL:
                imp.globalType.valType  = b.readValType();
                imp.globalType.mutable_ = b.readByte();
                break;
        }
        s.imports.push_back(imp);
    }
    b.offset = start + size;
    return s;
}

FunctionSection parseFunctionSection(ParserBuffer& b) {
    FunctionSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) s.typeIndices.push_back(b.readLEB());
    b.offset = start + size;
    return s;
}

TableSection parseTableSection(ParserBuffer& b) {
    TableSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        TableType t;
        t.elemType = b.readByte();
        t.limits = b.readLimits();
        s.tables.push_back(t);
    }
    b.offset = start + size;
    return s;
}

MemorySection parseMemorySection(ParserBuffer& b) {
    MemorySection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) s.memories.push_back(b.readLimits());
    b.offset = start + size;
    return s;
}

GlobalSection parseGlobalSection(ParserBuffer& b) {
    GlobalSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        Global g;
        g.valType = b.readValType();
        g.mutable_ = b.readByte();
        while (true) { // naive init expr parser (until 0x0B)
            u8 op = b.readByte();
            g.initExpr.push_back(op);
            if (op == 0x0B) break;
        }
        s.globals.push_back(g);
    }
    b.offset = start + size;
    return s;
}

ExportSection parseExportSection(ParserBuffer& b) {
    ExportSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        Export e;
        e.name = b.readName();
        e.kind = static_cast<ExportKind>(b.readByte());
        e.index = b.readLEB();
        s.exports.push_back(e);
    }
    b.offset = start + size;
    return s;
}

StartSection parseStartSection(ParserBuffer& b) {
    StartSection s;
    auto size = b.readLEB();
    s.functionIndex = b.readLEB();
    return s;
}

ElementSection parseElementSection(ParserBuffer& b) {
    ElementSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        Element e;
        e.tableIndex = b.readLEB();
        while (true) {
            u8 op = b.readByte();
            e.offsetExpr.push_back(op);
            if (op == 0x0B) break;
        }
        uint32_t fcount = b.readLEB();
        while (fcount--) e.functionIndices.push_back(b.readLEB());
        s.elements.push_back(e);
    }
    b.offset = start + size;
    return s;
}

CodeSection parseCodeSection(ParserBuffer& b) {
    CodeSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        FunctionBody body;
        uint32_t bodySize = b.readLEB();
        size_t bodyStart = b.offset;
        uint32_t localsCount = b.readLEB();
        while (localsCount--) {
            LocalEntry entry;
            entry.count = b.readLEB();
            entry.type = b.readValType();
            body.locals.push_back(entry);
        }
        while (b.offset < bodyStart + bodySize) {
            body.code.push_back(b.readByte());
        }
        s.bodies.push_back(body);
    }
    b.offset = start + size;
    return s;
}

DataSection parseDataSection(ParserBuffer& b) {
    DataSection s;
    auto size = b.readLEB();
    size_t start = b.offset;
    uint32_t count = b.readLEB();
    while (count--) {
        DataSegment d;
        d.memIndex = b.readLEB();
        while (true) {
            u8 op = b.readByte();
            d.offsetExpr.push_back(op);
            if (op == 0x0B) break;
        }
        uint32_t dataSize = b.readLEB();
        d.data.resize(dataSize);
        for (auto& byte : d.data) byte = b.readByte();
        s.segments.push_back(d);
    }
    b.offset = start + size;
    return s;
}

DataCountSection parseDataCountSection(ParserBuffer& b) {
    DataCountSection s;
    auto size = b.readLEB();
    s.count = b.readLEB();
    return s;
}

CustomSection parseCustomSection(ParserBuffer& b) {
    CustomSection section;
    auto sectionSize = b.readLEB();
    size_t start = b.offset;

    section.name = b.readName();

    size_t nameLen = section.name.size();
    size_t remaining = sectionSize - (b.offset - start);
    section.data.resize(remaining);
    for (size_t i = 0; i < remaining; ++i) {
        section.data[i] = b.readByte();
    }

    return section;
}

WasmModule ModuleParser::parseFromFile(std::string_view path) {
    std::ifstream input(path.data(), std::ios::binary);
    if (!input) throw std::runtime_error("Failed to open file");

    std::vector<uint8_t> fileBuffer(std::istreambuf_iterator<char>(input), {});
    ParserBuffer parserBuffer{.offset = MODULE_OFFSET, .buf = fileBuffer.data()};

    WasmModule module;

    while (parserBuffer.offset < fileBuffer.size()) {
        auto sectionId = static_cast<SectionType>(parserBuffer.readByte());

        switch (sectionId) {
            case SectionType::Custom:
                module.customSections.emplace_back(parseCustomSection(parserBuffer));
                break;
            case SectionType::Type:
                module.types = parseTypeSection(parserBuffer);
                break;
            case SectionType::Import:
                module.imports = parseImportSection(parserBuffer);
                break;
            case SectionType::Function:
                module.functions = parseFunctionSection(parserBuffer);
                break;
            case SectionType::Table:
                module.tables = parseTableSection(parserBuffer);
                break;
            case SectionType::Memory:
                module.memories = parseMemorySection(parserBuffer);
                break;
            case SectionType::Global:
                module.globals = parseGlobalSection(parserBuffer);
                break;
            case SectionType::Export:
                module.exports = parseExportSection(parserBuffer);
                break;
            case SectionType::Start:
                module.start = parseStartSection(parserBuffer);
                break;
            case SectionType::Element:
                module.elements = parseElementSection(parserBuffer);
                break;
            case SectionType::Code:
                module.code = parseCodeSection(parserBuffer);
                break;
            case SectionType::Data:
                module.data = parseDataSection(parserBuffer);
                break;
            case SectionType::DataCount:
                module.dataCount = parseDataCountSection(parserBuffer);
                break;
            default: {
                uint32_t sectionSize = parserBuffer.readLEB();
                parserBuffer.offset += sectionSize;
                break;
            }
        }
    }

    return module;
}
} //namespace omega::wass