
#include "util/module_parser.hpp"
#include "bytecode/test.hpp"

#include <cstdint>
#include <vector>

// Write an unsigned 32-bit integer in LEB128 into buf.
#include <cstdint>
#include <vector>

// Запись unsigned LEB128
static void write_u32_leb(uint32_t value, std::vector<uint8_t>& buf) {
    do {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (value) byte |= 0x80;
        buf.push_back(byte);
    } while (value);
}

/// codeInstr — это ваш чистый список opcodes: e.g.
///   { 0x41, 0x00,  // i32.const 0
///     0x21, 0x00,  // local.set 0
///     …
///     0x0B }       // end
///
/// Функция сама добавит «0 локальных» в начало тела.
std::vector<uint8_t> make_wasm_module_from_code(const std::vector<uint8_t>& codeInstr) {
    // Собираем полный вектор тела функции: локальные + инструкции
    std::vector<uint8_t> funcBody;
    // 0 групп локалов
    write_u32_leb(0, funcBody);
    // копируем ваши инструкции
    funcBody.insert(funcBody.end(), codeInstr.begin(), codeInstr.end());

    std::vector<uint8_t> out;
    out.reserve(128 + funcBody.size());

    // --- WASM header ---
    out.insert(out.end(), {0x00,0x61,0x73,0x6D});
    out.insert(out.end(), {0x01,0x00,0x00,0x00});

    // --- Type section (id=1) ---
    {
        std::vector<uint8_t> sec;
        write_u32_leb(1, sec);      // one signature
        sec.push_back(0x60);        // func form
        write_u32_leb(0, sec);      // param count = 0
        write_u32_leb(0, sec);      // result count = 0

        out.push_back(0x01);
        write_u32_leb(uint32_t(sec.size()), out);
        out.insert(out.end(), sec.begin(), sec.end());
    }

    // --- Function section (id=3) ---
    {
        std::vector<uint8_t> sec;
        write_u32_leb(1, sec);  // one function
        write_u32_leb(0, sec);  // type index = 0

        out.push_back(0x03);
        write_u32_leb(uint32_t(sec.size()), out);
        out.insert(out.end(), sec.begin(), sec.end());
    }

    // --- Code section (id=10) ---
    {
        std::vector<uint8_t> sec;
        write_u32_leb(1, sec);           // one function body
        write_u32_leb(uint32_t(funcBody.size()), sec);
        sec.insert(sec.end(), funcBody.begin(), funcBody.end());

        out.push_back(0x0A);
        write_u32_leb(uint32_t(sec.size()), out);
        out.insert(out.end(), sec.begin(), sec.end());
    }

    return out;
}


int main(int argc, char **argv) {

    omega::wass::ModuleParser parser("/home/asgrim/CLionProjects/OWasm-vm/wasm-examples/hello_import.wasm");
    auto module = parser.parseFromFile() ;
    omega::wass::bytecode_parser optimizer;
    auto v = optimizer.parse(module.codeSection[0].code);
//    auto с = make_wasm_module_from_code(v);

// Записать в файл
//    std::ofstream out("test.wasm", std::ios::binary);
//    out.write(reinterpret_cast<const char*>(с.data()), с.size());
    return 0;
}
