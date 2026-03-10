#ifndef MY_ISA_TYPES_H
#define MY_ISA_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

#ifdef ISA_32BIT
using Addr = u32;
#else
using Addr = u64;
#endif

struct StatusFlags {
    bool zero : 1;
    bool carry : 1;
    bool negative : 1;
    bool overflow : 1;

    void reset() {
        zero = false;
        carry = false;
        negative = false;
        overflow = false;
    }
};

struct MemoryAccessError : public std::runtime_error {
    MemoryAccessError(const std::string& msg) : std::runtime_error(msg) {}
};

struct InvalidInstructionError : public std::runtime_error {
    InvalidInstructionError(const std::string& msg) : std::runtime_error(msg) {}
};

#endif // MY_ISA_TYPES_H