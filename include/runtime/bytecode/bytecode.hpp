
#ifndef OWASM_VM_BYTECODE_HPP
#define OWASM_VM_BYTECODE_HPP
#include "data/types.hpp"

namespace omega::wass::runtime {

enum Bytecode : u8 {
    // Control instructions
    unreachable       = 0x00,
    nop               = 0x01,
    block             = 0x02,
    loop              = 0x03,
    if_               = 0x04,
    else_             = 0x05,
    end               = 0x0B,
    br                = 0x0C,
    br_if             = 0x0D,
    br_table          = 0x0E,
    return_           = 0x0F, // "return" is a keyword in many languages

    // Calls & references
    call              = 0x10,
    return_call       = 0x12,
    return_call_indirect = 0x13,
    call_ref          = 0x14,

    // Parametric
    drop              = 0x1A,
    select            = 0x1B,
    select_t          = 0x1C, // select with type list

    // Local/global
    local_get         = 0x20,
    local_set         = 0x21,
    local_tee         = 0x22,
    global_get        = 0x23,
    global_set        = 0x24,

    // Tables
    table_get         = 0x25,
    table_set         = 0x26,

    // Memory (loads)
    i32_load          = 0x28,
    i64_load          = 0x29,
    f32_load          = 0x2A,
    f64_load          = 0x2B,
    i32_load8_s       = 0x2C,
    i32_load8_u       = 0x2D,
    i32_load16_s      = 0x2E,
    i32_load16_u      = 0x2F,
    i64_load8_s       = 0x30,
    i64_load8_u       = 0x31,
    i64_load16_s      = 0x32,
    i64_load16_u      = 0x33,
    i64_load32_s      = 0x34,
    i64_load32_u      = 0x35,

    // Memory (stores)
    i32_store         = 0x36,
    i64_store         = 0x37,
    f32_store         = 0x38,
    f64_store         = 0x39,
    i32_store8        = 0x3A,
    i32_store16       = 0x3B,
    i64_store8        = 0x3C,
    i64_store16       = 0x3D,
    i64_store32       = 0x3E,

    // Memory size/grow
    memory_size       = 0x3F,
    memory_grow       = 0x40,

    // Constants
    i32_const         = 0x41,
    i64_const         = 0x42,
    f32_const         = 0x43,
    f64_const         = 0x44,

    // Simple tests
    i32_eqz           = 0x45,
    i32_eq            = 0x46,
    i32_ne            = 0x47,
    i32_lt_s          = 0x48,
    i32_lt_u          = 0x49,
    i32_gt_s          = 0x4A,
    i32_gt_u          = 0x4B,
    i32_le_s          = 0x4C,
    i32_le_u          = 0x4D,
    i32_ge_s          = 0x4E,
    i32_ge_u          = 0x4F,

    i64_eqz           = 0x50,
    i64_eq            = 0x51,
    i64_ne            = 0x52,
    i64_lt_s          = 0x53,
    i64_lt_u          = 0x54,
    i64_gt_s          = 0x55,
    i64_gt_u          = 0x56,
    i64_le_s          = 0x57,
    i64_le_u          = 0x58,
    i64_ge_s          = 0x59,
    i64_ge_u          = 0x5A,

    // Floating-point comparisons
    f32_eq            = 0x5B,
    f32_ne            = 0x5C,
    f32_lt            = 0x5D,
    f32_gt            = 0x5E,
    f32_le            = 0x5F,
    f32_ge            = 0x60,
    f64_eq            = 0x61,
    f64_ne            = 0x62,
    f64_lt            = 0x63,
    f64_gt            = 0x64,
    f64_le            = 0x65,
    f64_ge            = 0x66,

    // Integer arithmetic
    i32_clz           = 0x67,
    i32_ctz           = 0x68,
    i32_popcnt        = 0x69,
    i32_add           = 0x6A,
    i32_sub           = 0x6B,
    i32_mul           = 0x6C,
    i32_div_s         = 0x6D,
    i32_div_u         = 0x6E,
    i32_rem_s         = 0x6F,
    i32_rem_u         = 0x70,
    i32_and           = 0x71,
    i32_or            = 0x72,
    i32_xor           = 0x73,
    i32_shl           = 0x74,
    i32_shr_s         = 0x75,
    i32_shr_u         = 0x76,
    i32_rotl          = 0x77,
    i32_rotr          = 0x78,

    i64_clz           = 0x79,
    i64_ctz           = 0x7A,
    i64_popcnt        = 0x7B,
    i64_add           = 0x7C,
    i64_sub           = 0x7D,
    i64_mul           = 0x7E,
    i64_div_s         = 0x7F,
    i64_div_u         = 0x80,
    i64_rem_s         = 0x81,
    i64_rem_u         = 0x82,
    i64_and           = 0x83,
    i64_or            = 0x84,
    i64_xor           = 0x85,
    i64_shl           = 0x86,
    i64_shr_s         = 0x87,
    i64_shr_u         = 0x88,
    i64_rotl          = 0x89,
    i64_rotr          = 0x8A,

    // Floating-point arithmetic
    f32_abs           = 0x8B,
    f32_neg           = 0x8C,
    f32_ceil         = 0x8D,
    f32_floor        = 0x8E,
    f32_trunc        = 0x8F,
    f32_nearest      = 0x90,
    f32_sqrt         = 0x91,
    f32_add          = 0x92,
    f32_sub          = 0x93,
    f32_mul          = 0x94,
    f32_div          = 0x95,
    f32_min          = 0x96,
    f32_max          = 0x97,
    f32_copysign     = 0x98,

    f64_abs           = 0x99,
    f64_neg           = 0x9A,
    f64_ceil          = 0x9B,
    f64_floor         = 0x9C,
    f64_trunc         = 0x9D,
    f64_nearest       = 0x9E,
    f64_sqrt          = 0x9F,
    f64_add           = 0xA0,
    f64_sub           = 0xA1,
    f64_mul           = 0xA2,
    f64_div           = 0xA3,
    f64_min           = 0xA4,
    f64_max           = 0xA5,
    f64_copysign      = 0xA6,

    // Conversions & reinterpretations
    i32_wrap_i64        = 0xA7,
    i32_trunc_s_f32     = 0xA8,
    i32_trunc_u_f32     = 0xA9,
    i32_trunc_s_f64     = 0xAA,
    i32_trunc_u_f64     = 0xAB,
    i64_extend_s_i32    = 0xAC,
    i64_extend_u_i32    = 0xAD,
    i64_trunc_s_f32     = 0xAE,
    i64_trunc_u_f32     = 0xAF,
    i64_trunc_s_f64     = 0xB0,
    i64_trunc_u_f64     = 0xB1,
    f32_convert_s_i32   = 0xB2,
    f32_convert_u_i32   = 0xB3,
    f32_convert_s_i64   = 0xB4,
    f32_convert_u_i64   = 0xB5,
    f32_demote_f64      = 0xB6,
    f64_convert_s_i32   = 0xB7,
    f64_convert_u_i32   = 0xB8,
    f64_convert_s_i64   = 0xB9,
    f64_convert_u_i64   = 0xBA,
    f64_promote_f32     = 0xBB,
    i32_reinterpret_f32 = 0xBC,
    i64_reinterpret_f64 = 0xBD,
    f32_reinterpret_i32 = 0xBE,
    f64_reinterpret_i64 = 0xBF
};




}

#endif //OWASM_VM_BYTECODE_HPP
