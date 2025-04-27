
#ifndef OWASM_VM_BYTECODE_OPTIMIZER_HPP
#define OWASM_VM_BYTECODE_OPTIMIZER_HPP

#include "data/module_struct.hpp"
#include "bytecode.hpp"
#include <deque>
#include <array>
namespace omega::wass {

namespace impl {
    class Instr {
        Bytecode op;
        //index in bytecode array
        u32 ind;
        union {
            std::vector<u8> args;
            //stack of block if instr is a block
            std::deque<Instr> blocks;
        } data;
    };
}

static constexpr std::array<uint8_t, 256> kWasmImmediateCount = []{
    std::array<uint8_t, 256> arr{};
    // Control instructions
    arr[0x02] = 1;   // block <blocktype>
    arr[0x03] = 1;   // loop  <blocktype>
    arr[0x04] = 1;   // if    <blocktype>
    arr[0x06] = 1;   // try   <blocktype>  (exception handling)
    arr[0x08] = 1;   // throw <tagidx>
    arr[0x09] = 1;   // rethrow <labelidx>
    arr[0x0C] = 1;   // br    <labelidx>
    arr[0x0D] = 1;   // br_if <labelidx>
    arr[0x0E] = 0xFF;// br_table <vec<labelidx>> + <labelidx> (variable)

    // Calls & references
    arr[0x10] = 1;   // call <funcidx>
    arr[0x12] = 1;   // return_call <funcidx>
    arr[0x13] = 2;   // return_call_indirect <typeidx> + <tableidx>
    arr[0x14] = 1;   // call_ref <reftype>

    // Parametric
    arr[0x1C] = 1;   // select_t <resulttype_vec>

    // Local / global
    arr[0x20] = 1;   // local.get <localidx>
    arr[0x21] = 1;   // local.set <localidx>
    arr[0x22] = 1;   // local.tee <localidx>
    arr[0x23] = 1;   // global.get <globalidx>
    arr[0x24] = 1;   // global.set <globalidx>

    // Tables
    arr[0x25] = 1;   // table.get <tableidx>
    arr[0x26] = 1;   // table.set <tableidx>

    // Memory loads (align + offset)
    for (uint8_t op = 0x28; op <= 0x2F; ++op) arr[op] = 2;
    for (uint8_t op = 0x30; op <= 0x35; ++op) arr[op] = 2;

    // Memory stores (align + offset)
    for (uint8_t op = 0x36; op <= 0x3B; ++op) arr[op] = 2;
    for (uint8_t op = 0x3C; op <= 0x3E; ++op) arr[op] = 2;

    // Memory size / grow (reserved = 0)
    arr[0x3F] = 1;   // memory.size <0>
    arr[0x40] = 1;   // memory.grow <0>

    // Constants
    arr[0x41] = 1;   // i32.const <varint32>
    arr[0x42] = 1;   // i64.const <varint64>
    arr[0x43] = 1;   // f32.const <IEEE754>
    arr[0x44] = 1;   // f64.const <IEEE754>

    return arr;
}();
class BytecodeOptimizer {};

}
#endif //OWASM_VM_BYTECODE_OPTIMIZER_HPP
