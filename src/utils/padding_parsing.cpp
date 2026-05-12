#include "padding_parsing.h"

std::vector<uint8_t> pad_and_parse(const std::string& message) {
    std::vector<uint8_t> padded;
    
    // 1. Chuyển đổi message gốc thành mảng byte
    for (char c : message) {
        padded.push_back(static_cast<uint8_t>(c));
    }

    // Lưu lại chiều dài gốc tính bằng BIT
    uint64_t original_bit_len = message.length() * 8;

    // 2. Thêm bit '1' (Tương đương với byte 0x80 hay 10000000)
    padded.push_back(0x80);

    // 3. Thêm các bit '0' cho đến khi độ dài mảng byte chia cho 128 dư 112 
    // (Tương đương 896 bit mod 1024 bit)
    while ((padded.size() % 128) != 112) {
        padded.push_back(0x00);
    }

    // 4. Thêm 128-bit (16 bytes) biểu diễn chiều dài gốc của message.
    // Vì giới hạn của uint64_t là quá đủ cho văn bản thông thường, 
    // 8 byte đầu (64 bit cao) của phần chiều dài này sẽ luôn là 0x00.
    for (int i = 0; i < 8; ++i) {
        padded.push_back(0x00);
    }
    
    // 8 byte sau (64 bit thấp) lưu giá trị original_bit_len (Big-Endian)
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<uint8_t>((original_bit_len >> (i * 8)) & 0xFF));
    }

    return padded;
}