#ifndef UTILS_BIT_OPERATIONS_H
#define UTILS_BIT_OPERATIONS_H

#include <stdint.h>

// Hàm Rotate Right (Dịch xoay phải) cho 64-bit
inline uint64_t rotr(uint64_t x, uint32_t n) {
    return (x >> n) | (x << (64 - n));
}

// Hàm Shift Right (Dịch phải) cho 64-bit
inline uint64_t shr(uint64_t x, uint32_t n) {
    return (x >> n);
}

#endif // UTILS_BIT_OPERATIONS_H