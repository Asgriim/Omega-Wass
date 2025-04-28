#ifndef OWASM_VM_BYTECODE_PARSER_HPP
#define OWASM_VM_BYTECODE_PARSER_HPP

#include "data/types.hpp"
#include "bytecode.hpp"
#include <vector>
#include <array>
#include <stdexcept>

namespace omega::wass {

// Типы immediates
enum class arg_type : u8 { none, varuint32, fixed32, fixed64 };

// Описание одной инструкции после парсинга
struct instr {
    Bytecode         opcode;        // код операции
    u32              offset;        // байтовый офсет начала инструкции
    std::vector<u32> immediates;    // разобранные аргументы
    u32              matching_end = 0; // офсет соответствующего end для block/loop/if
};

// Глобальные constexpr таблицы immediates
static constexpr std::array<u8,256> arg_count = []{
    std::array<u8,256> a{};
    a[(u8)Bytecode::block] = 1;
    a[(u8)Bytecode::loop]  = 1;
    a[(u8)Bytecode::if_]   = 1;
    a[(u8)Bytecode::br]    = 1;
    a[(u8)Bytecode::br_if] = 1;
    a[(u8)Bytecode::local_get] = 1;
    a[(u8)Bytecode::local_set] = 1;
    a[(u8)Bytecode::local_tee] = 1;
    a[(u8)Bytecode::global_get] = 1;
    a[(u8)Bytecode::global_set] = 1;
    a[(u8)Bytecode::table_get] = 1;
    a[(u8)Bytecode::table_set] = 1;
    a[(u8)Bytecode::call] = 1;
    a[(u8)Bytecode::return_call] = 1;
    a[(u8)Bytecode::return_call_indirect] = 2;
    a[(u8)Bytecode::call_ref] = 1;
    for (u8 op = (u8)Bytecode::i32_load; op <= (u8)Bytecode::i64_load32_u; ++op) a[op] = 2;
    for (u8 op = (u8)Bytecode::i32_store; op <= (u8)Bytecode::i64_store32; ++op) a[op] = 2;
    a[(u8)Bytecode::memory_size] = 1;
    a[(u8)Bytecode::memory_grow] = 1;
    a[(u8)Bytecode::i32_const] = 1;
    a[(u8)Bytecode::i64_const] = 1;
    a[(u8)Bytecode::f32_const] = 1;
    a[(u8)Bytecode::f64_const] = 1;
    return a;
}();
static constexpr int max_args = 2;
static constexpr std::array<std::array<arg_type,max_args>,256> arg_types = []{
    std::array<std::array<arg_type,max_args>,256> t{};
    auto set = [&](Bytecode bc, arg_type a0, arg_type a1=arg_type::none) {
        t[(u8)bc][0] = a0;
        t[(u8)bc][1] = a1;
    };
    set(Bytecode::block, arg_type::varuint32);
    set(Bytecode::loop, arg_type::varuint32);
    set(Bytecode::if_, arg_type::varuint32);
    set(Bytecode::br, arg_type::varuint32);
    set(Bytecode::br_if, arg_type::varuint32);
    set(Bytecode::local_get, arg_type::varuint32);
    set(Bytecode::local_set, arg_type::varuint32);
    set(Bytecode::local_tee, arg_type::varuint32);
    set(Bytecode::global_get, arg_type::varuint32);
    set(Bytecode::global_set, arg_type::varuint32);
    set(Bytecode::table_get, arg_type::varuint32);
    set(Bytecode::table_set, arg_type::varuint32);
    set(Bytecode::call, arg_type::varuint32);
    set(Bytecode::return_call, arg_type::varuint32);
    set(Bytecode::return_call_indirect, arg_type::varuint32, arg_type::varuint32);
    set(Bytecode::call_ref, arg_type::varuint32);
    for (u8 op = (u8)Bytecode::i32_load; op <= (u8)Bytecode::i64_load32_u; ++op)
        set((Bytecode)op, arg_type::varuint32, arg_type::varuint32);
    for (u8 op = (u8)Bytecode::i32_store; op <= (u8)Bytecode::i64_store32; ++op)
        set((Bytecode)op, arg_type::varuint32, arg_type::varuint32);
    set(Bytecode::memory_size, arg_type::varuint32);
    set(Bytecode::memory_grow, arg_type::varuint32);
    set(Bytecode::i32_const, arg_type::varuint32);
    set(Bytecode::i64_const, arg_type::varuint32);
    set(Bytecode::f32_const, arg_type::fixed32);
    set(Bytecode::f64_const, arg_type::fixed64);
    return t;
}();

class bytecode_parser {
public:
    // парсинг байт-кода в массив instr
    std::vector<instr> parse(const std::vector<u8>& code) {
        std::vector<instr> insts;
        insts.reserve(code.size()/2);
        std::vector<size_t> block_stack;

        size_t pos = 0;
        while (pos < code.size()) {
            instr ins;
            ins.offset = u32(pos);
            ins.opcode = Bytecode(code[pos++]);

            // чтение immediates по таблице
            u8 cnt = arg_count[(u8)ins.opcode];
            for (u8 i = 0; i < cnt; ++i) {
                auto at = arg_types[(u8)ins.opcode][i];
                if (at == arg_type::varuint32) {
                    size_t len;
                    u32 val = read_varuint32(code, pos, len);
                    ins.immediates.push_back(val);
                    pos += len;
                } else if (at == arg_type::fixed32) {
                    if (pos + 4 > code.size()) throw std::runtime_error("unexpected end reading fixed32");
                    u32 val = u32(code[pos]) | u32(code[pos+1])<<8 |
                              u32(code[pos+2])<<16 | u32(code[pos+3])<<24;
                    ins.immediates.push_back(val);
                    pos += 4;
                } else if (at == arg_type::fixed64) {
                    if (pos + 8 > code.size()) throw std::runtime_error("unexpected end reading fixed64");
                    u32 val = u32(code[pos]) | u32(code[pos+1])<<8 |
                              u32(code[pos+2])<<16 | u32(code[pos+3])<<24;
                    ins.immediates.push_back(val);
                    pos += 8;
                }
            }

            // отслеживание блоков и корректный unmatched end
            if (ins.opcode == Bytecode::block || ins.opcode == Bytecode::loop || ins.opcode == Bytecode::if_) {
                block_stack.push_back(insts.size());
            } else if (ins.opcode == Bytecode::end) {
                // allow final end without matching block
                if (block_stack.empty()) {
                    if (pos != code.size())
                        throw std::runtime_error("unmatched end bytecode");
                } else {
                    size_t open_idx = block_stack.back();
                    block_stack.pop_back();
                    insts[open_idx].matching_end = ins.offset;
                    ins.matching_end = ins.offset;
                }
            }

            insts.push_back(std::move(ins));
        }

        if (!block_stack.empty())
            throw std::runtime_error("unclosed block/loop/if at end of bytecode");

        return insts;
    }

private:
    // чтение varuint32 LEB128, возвращает значение и длину
    static u32 read_varuint32(const std::vector<u8>& code, size_t pos, size_t& len_out) {
        u32 result = 0;
        unsigned shift = 0;
        size_t start = pos;
        while (true) {
            if (pos >= code.size())
                throw std::runtime_error("unexpected end reading LEB128");
            u8 byte = code[pos++];
            result |= u32(byte & 0x7F) << shift;
            if (!(byte & 0x80)) break;
            shift += 7;
        }
        len_out = pos - start;
        return result;
    }
};

} // namespace omega::wass

#endif // OWASM_VM_BYTECODE_PARSER_HPP
