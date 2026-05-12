#ifndef SHA512_INIT_H
#define SHA512_INIT_H

#include "../utils/common.h"
#include <array>

namespace sha512 {
    // Khai báo extern để các file khác có thể truy cập trực tiếp mảng này
    extern const std::array<uint64_t, 80> K;
    extern const std::array<uint64_t, 8> H0;
}

#endif // SHA512_INIT_H