#ifndef OWASM_VM_BUF_READER_HPP
#define OWASM_VM_BUF_READER_HPP

#include <fstream>
#include <cstring>
#include <vector>
#include "util/util.hpp"

namespace omega::wass::util {
constexpr size_t MODULE_OFFSET = 8; // 4 bytes magic + 4 bytes version

class BufReader {
public:
    explicit BufReader(std::ifstream stream);

    [[nodiscard]]
    u8* get() const noexcept { return buf_ptr_ + offset_; }

    template<typename T>
    T read() {
        T t;
        std::memcpy(&t, get(), sizeof(T));
        offset_ += sizeof(T);
        return t;
    }

    i64 readLeb128();
    u64 readULeb128();
    std::string readStr();

    template<typename T>
    void next() {
        offset_ += sizeof(T);
    }

    void next(i64 off) {
        offset_ += off;
    }
    bool isEnd() {
        return offset_ >= buff_.size();
    }
private:
    std::vector<u8> buff_;
    uint8_t *buf_ptr_;
    size_t offset_ = MODULE_OFFSET;
};

}
#endif //OWASM_VM_BUF_READER_HPP
