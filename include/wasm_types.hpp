#ifndef OWASM_VM_WASM_TYPES_HPP
#define OWASM_VM_WASM_TYPES_HPP

#include <optional>
#include <vector>
#include <string>
#include <cstdint>

namespace omega::wass {

using u8  = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;
using i64 = int64_t;

enum ValType : int8_t {
    I32 = 0x7F,
    I64 = 0x7E,
    F32 = 0x7D,
    F64 = 0x7C,
};

enum class SectionType : uint8_t {
    Custom       = 0,  // Имя + произвольные данные
    Type         = 1,  // Функциональные сигнатуры
    Import       = 2,  // Импорты (функции, память, таблицы и т.п.)
    Function     = 3,  // Индексы функций (внутренних)
    Table        = 4,  // Описание таблиц (обычно для funcref)
    Memory       = 5,  // Описание памяти
    Global       = 6,  // Глобальные переменные
    Export       = 7,  // Экспорт символов
    Start        = 8,  // Индекс стартовой функции
    Element      = 9,  // Инициализация таблиц
    Code         = 10, // Тела функций
    Data         = 11, // Инициализация памяти
    DataCount    = 12  // Число data-сегментов (опционально)
};


struct Limits {
    u32 min;
    u32 max;
};

struct FuncType {
    std::vector<ValType> params;
    std::vector<ValType> results;
};

struct CustomSection {
    std::string name;
    std::vector<u8> data;
};


struct TypeSection {
    std::vector<FuncType> types;
};


enum ImportKind : u8 {
    FUNC = 0x00,
    TABLE = 0x01,
    MEMORY = 0x02,
    GLOBAL = 0x03
};

struct Import {
    std::string module;
    std::string name;
    ImportKind kind;
    union {
        u32 typeIndex;       // for function
        Limits tableLimits;  // for table
        Limits memLimits;    // for memory
        struct {
            ValType valType;
            u8 mutable_;
        } globalType;
    } ;
};

struct ImportSection {
    std::vector<Import> imports;
};



struct FunctionSection {
    std::vector<u32> typeIndices;  // Index into type section
};


struct TableType {
    u8 elemType; // usually 0x70
    Limits limits;
};

struct TableSection {
    std::vector<TableType> tables;
};

struct MemorySection {
    std::vector<Limits> memories;
};


struct Global {
    ValType valType;
    u8 mutable_;
    std::vector<u8> initExpr; // raw bytes of init expr (can be parsed later)
};

struct GlobalSection {
    std::vector<Global> globals;
};

enum ExportKind : u8 {
    FUNC_EXP = 0x00,
    TABLE_EXP = 0x01,
    MEM_EXP = 0x02,
    GLOBAL_EXP = 0x03
};

struct Export {
    std::string name;
    ExportKind kind;
    u32 index;
};

struct ExportSection {
    std::vector<Export> exports;
};


struct StartSection {
    u32 functionIndex;
};


struct Element {
    u32 tableIndex;
    std::vector<u8> offsetExpr;
    std::vector<u32> functionIndices;
};

struct ElementSection {
    std::vector<Element> elements;
};


struct LocalEntry {
    u32 count;
    ValType type;
};

struct FunctionBody {
    std::vector<LocalEntry> locals;
    std::vector<u8> code;  // function body (instructions + 0x0B)
};

struct CodeSection {
    std::vector<FunctionBody> bodies;
};



struct DataSegment {
    u32 memIndex;
    std::vector<u8> offsetExpr;
    std::vector<u8> data;
};

struct DataSection {
    std::vector<DataSegment> segments;
};

struct DataCountSection {
    u32 count;
};


struct WasmModule {
    std::vector<CustomSection> customSections;
    std::optional<TypeSection> types;
    std::optional<ImportSection> imports;
    FunctionSection functions;
    std::optional<TableSection> tables;
    std::optional<MemorySection> memories;
    std::optional<GlobalSection> globals;
    std::optional<ExportSection> exports;
    std::optional<StartSection> start;
    std::optional<ElementSection> elements;
    std::optional<CodeSection> code;
    std::optional<DataSection> data;
    std::optional<DataCountSection> dataCount;
};


} //namespace omega::wass
#endif //OWASM_VM_WASM_TYPES_HPP
