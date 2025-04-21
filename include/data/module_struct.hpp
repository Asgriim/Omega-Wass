#ifndef OWASM_VM_MODULE_STRUCT_HPP
#define OWASM_VM_MODULE_STRUCT_HPP

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

}

#endif //OWASM_VM_MODULE_STRUCT_HPP
