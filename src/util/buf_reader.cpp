#include "util/buf_reader.hpp"
namespace omega::wass::util {

BufReader::BufReader(std::ifstream stream): buff_(std::istreambuf_iterator<char>(stream), {}) {
    buf_ptr_ = buff_.data();
}

i64 BufReader::readLeb128() {
    i64 result = 0;
    i32 shift = 0;
    uint8_t byte = 0;

    for (i32 i = 0; i < MAX_LEB128_BYTES; i++, next<u8>()) {
        byte = *get();
        result |= i64 (byte & 0x7F) << shift;
        shift += 7;
        if ((byte & 0x80) == 0) {
            // Last byte—perform sign extension if needed
            if ((byte & 0x40) && shift < 64) {
                result |= - (i64 (1) << shift);
            }

            next<u8>();
            return result;
        }
    }
    throw std::overflow_error("SLEB128 exceeds maximum length for int64_t");
}

std::string BufReader::readStr() {
    i64 len = readLeb128();
    std::string str(reinterpret_cast<char*>(get()), len);
    offset_ += len;
    return str;
}

u64 BufReader::readULeb128() {
    u64 result = 0;
    unsigned shift = 0;

    for (i32 i = 0; i < MAX_LEB128_BYTES; i++) {
        u8 byte = *get();
        result |= u64(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            next<u8>();
            return result;
        }
        shift += 7;
    }
    throw std::overflow_error("ULEB128 exceeds maximum length for uint64_t");
}
}