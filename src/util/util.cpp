#include "util/util.hpp"
#include "algorithm"

namespace omega::wass::util {

//todo maybe delete
i64 readULEB128(const u8* ptr) {
    uint64_t result = 0;
    unsigned shift = 0;

    for (unsigned i = 0; i < MAX_LEB128_BYTES; i++) {
        uint8_t byte = *ptr++;
        result |= uint64_t(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            return result;
        }
        shift += 7;
    }
    throw std::overflow_error("ULEB128 exceeds maximum length for uint64_t");
}

i64 readLEB128(const u8* ptr) {
    int64_t result = 0;
    unsigned shift = 0;
    uint8_t byte = 0;

    for (unsigned i = 0; i < MAX_LEB128_BYTES; i++) {
        byte = *ptr++;
        result |= int64_t(byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            // Last byte—perform sign extension if needed
            if ((byte & 0x40) && shift < 64) {
                result |= - (int64_t(1) << shift);
            }
            return result;
        }
    }
    throw std::overflow_error("SLEB128 exceeds maximum length for int64_t");
}


void trim(std::string &s) {
    const auto not_space = [](char c){ return !std::isspace(static_cast<unsigned char>(c)); };
    auto left  = std::find_if(s.begin(), s.end(),     not_space);
    auto right = std::find_if(s.rbegin(), s.rend(),   not_space).base();
    if (left < right)
        s = std::string(left, right);
    else
        s.clear();
}


std::pair<std::string, std::string> parse_call(const std::string &input) {
    auto open  = input.find('(');
    auto close = input.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open) {
        throw std::invalid_argument("Invalid format: expected 'name(signature)'");
    }

    std::string name   = input.substr(0, open);
    std::string inside = input.substr(open + 1, close - open - 1);

    trim(name);
    trim(inside);

    return {name, inside};
}

}
