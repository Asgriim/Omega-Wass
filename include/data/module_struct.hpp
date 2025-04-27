#ifndef OWASM_VM_MODULE_STRUCT_HPP
#define OWASM_VM_MODULE_STRUCT_HPP

#include <vector>
#include <string>
#include "data/types.hpp"

namespace omega::wass::module {
enum class SectionType : u8 {
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

struct FuncSignature {
    std::vector<ValType> params;
    std::vector<ValType> results;
};

struct CustomSection {
    std::string name;
    std::vector<u8> data;
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


struct TableType {
    u8 elemType; // usually 0x70
    Limits limits;
};

struct Global {
    ValType valType;
    MutableType mutable_;
    std::vector<u8> initExpr; // raw bytes of init expr (can be parsed later)
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

struct StartSection {
    u32 functionIndex;
};

struct Element {
    u32 tableIndex;
    std::vector<u8> offsetExpr;
    std::vector<u32> functionIndices;
};

struct LocalEntry {
    u32 count;
    ValType type;
};

struct FunctionBody {
    std::vector<LocalEntry> locals;
    std::vector<u8> code;  // function body (instructions + 0x0B)
};

struct DataSegment {
    u32 memIndex;
    std::vector<u8> offsetExpr;
    std::vector<u8> data;
};

struct DataCountSection {
    u32 count;
};

struct FuncIndex {
    i32 ind;
};

struct WasmModule {
    std::vector<CustomSection> customSection;
    std::vector<FuncSignature> typesSection;
    std::vector<Import> importSection;
    std::vector<FuncIndex> functionSection;  // Index into type section
    std::vector<TableType>  tableSection;
    std::vector<Limits> memorySection;
    std::vector<Global> globalSection;
    std::vector<Export> exportSection;
    StartSection startSection;
    std::vector<Element> elementSection;
    std::vector<FunctionBody> codeSection;
    std::vector<DataSegment> dataSection;
    DataCountSection dataCountSection;
};


}

#endif //OWASM_VM_MODULE_STRUCT_HPP
