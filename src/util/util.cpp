#include "util/util.hpp"




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
}
