#ifndef SHA512_CORE_H
#define SHA512_CORE_H

#include "../utils/common.h"

namespace sha512 {
    // Hàm duy nhất được phép gọi từ giao diện/bên ngoài.
    // Nhận mảng byte đã được padding chuẩn xác, trả về chuỗi Hex SHA-512 (chữ thường).
    std::string hash_padded_data(const std::vector<uint8_t>& padded_data);
}

#endif // SHA512_CORE_H