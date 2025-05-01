//
// Created by asgrim on 13.04.25.
//

#ifndef OWASM_VM_UTIL_HPP
#define OWASM_VM_UTIL_HPP

#include <cstdint>
#include <stdexcept>
#include "data/types.hpp"

namespace omega::wass::util {
static constexpr u32 MAX_LEB128_BYTES = (64 + 6) / 7;  // = 10

i64 readULEB128(const u8* ptr);
i64 readLEB128(const u8* ptr);
std::pair<std::string, std::string> parse_call(const std::string &input);
void trim(std::string &s);
}
#endif //OWASM_VM_UTIL_HPP
