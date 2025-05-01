#ifndef OWASM_VM_TYPES_HPP
#define OWASM_VM_TYPES_HPP


#include <cstdint>
namespace omega::wass {
using u8  = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

enum ValType : u8 {
    I32   = 0x7F,
    I64   = 0x7E,
    F32   = 0x7D,
    F64   = 0x7C,
    BLOCK = 0x40,
    REF   = 0x78
};

enum MutableType : u8 {
    CONST = 0x00,
    MUT   = 0x01
};

namespace native {
enum FuncParams : u8 {
    REF = '*',
    INT = 'I',
    FLOAT = 'F'
};
}

// Reference Types
namespace reftype {
constexpr u8 funcref    = 0x70;
constexpr u8 externref  = 0x6F;
}

// Function Type
namespace functype {
constexpr u8 func       = 0x60;
}

// Block Type
namespace blocktype {
constexpr u8 empty      = 0x40; // no result
}

// External Kind for imports/exports
namespace external {
constexpr u8 function   = 0x00;
constexpr u8 table      = 0x01;
constexpr u8 memory     = 0x02;
constexpr u8 global     = 0x03;
// Optional: tag for exception handling
constexpr u8 tag        = 0x04;
}

// Global Mutability
namespace mutability {
constexpr u8 const_     = 0x00;
constexpr u8 var        = 0x01;
}

namespace lims {
constexpr u8 min_flag = 0x00;
constexpr u8 max_flag = 0x01;
}

namespace expr {
constexpr u8 end = 0x0B;
}
}
#endif //OWASM_VM_TYPES_HPP
