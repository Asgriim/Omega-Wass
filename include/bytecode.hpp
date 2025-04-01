#ifndef OWASM_VM_BYTECODE_HPP
#define OWASM_VM_BYTECODE_HPP

#include "runtime_structures.hpp"
#include <array>
namespace omega::wass {


enum WasmOpcode : uint8_t {
    UNREACHABLE = 0x00,
    NOP = 0x01,
    BLOCK = 0x02,
    LOOP = 0x03,
    IF = 0x04,
    ELSE = 0x05,
    END = 0x0b,
    BR_IF = 0x0d,
    CALL = 0x10,
    DROP = 0x1a,
    LOCAL_GET = 0x20,
    LOCAL_SET = 0x21,
    I32_CONST = 0x41,
    I32_ADD = 0x6a,
    I32_REM_U = 0x70,
    I32_EQZ = 0x45,
    I32_LE_S = 0x4c,
};

using InstructionHandler = void(*)(Frame&, ExecCtx*);
inline std::array<InstructionHandler, 256> dispatch_table = {};


 void init_dispatch_table();


void unreachable(Frame &curr_frame, ExecCtx *ctx);
void nop(Frame &curr_frame, ExecCtx *ctx);
void block(Frame &curr_frame, ExecCtx *ctx);
void loop(Frame &curr_frame, ExecCtx *ctx);
void if_op(Frame &curr_frame, ExecCtx *ctx);
void else_op(Frame &curr_frame, ExecCtx *ctx);
void end(Frame &curr_frame, ExecCtx *ctx);
void br_if(Frame &curr_frame, ExecCtx *ctx);
void call(Frame &curr_frame, ExecCtx *ctx);
void drop(Frame &curr_frame, ExecCtx *ctx);
void local_get(Frame &curr_frame, ExecCtx *ctx);
void local_set(Frame &curr_frame, ExecCtx *ctx);
void i32_const(Frame &curr_frame, ExecCtx *ctx);
void i32_add(Frame &curr_frame, ExecCtx *ctx);
void i32_rem_u(Frame &curr_frame, ExecCtx *ctx);
void i32_eqz(Frame &curr_frame, ExecCtx *ctx);
void i32_le_s(Frame &curr_frame, ExecCtx *ctx);

}
#endif //OWASM_VM_BYTECODE_HPP
