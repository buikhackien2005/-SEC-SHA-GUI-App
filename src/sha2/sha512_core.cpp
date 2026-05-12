#include "sha512_core.h"
#include "sha512_init.h"
#include "../utils/bit_operations.h"

namespace sha512 {

namespace { 
    // Giấu kín các hàm logic lõi vào anonymous namespace. 
    // Từ khóa inline ép trình biên dịch gắn trực tiếp code vào vòng lặp để triệt tiêu chi phí gọi hàm.
    inline uint64_t Ch(uint64_t e, uint64_t f, uint64_t g) { return (e & f) ^ (~e & g); }
    inline uint64_t Maj(uint64_t a, uint64_t b, uint64_t c) { return (a & b) ^ (a & c) ^ (b & c); }
    inline uint64_t BigSigma0(uint64_t a) { return rotr(a, 28) ^ rotr(a, 34) ^ rotr(a, 39); }
    inline uint64_t BigSigma1(uint64_t e) { return rotr(e, 14) ^ rotr(e, 18) ^ rotr(e, 41); }
    inline uint64_t SmallSigma0(uint64_t x) { return rotr(x, 1) ^ rotr(x, 8) ^ shr(x, 7); }
    inline uint64_t SmallSigma1(uint64_t x) { return rotr(x, 19) ^ rotr(x, 61) ^ shr(x, 6); }
}

std::string hash_padded_data(const std::vector<uint8_t>& padded_data) {
    // Khởi tạo thanh ghi bằng mảng H0
    uint64_t H[8];
    for (int i = 0; i < 8; ++i) H[i] = H0[i];

    // Xử lý theo từng block 1024-bit (128 bytes)
    size_t num_blocks = padded_data.size() / 128;
    for (size_t block = 0; block < num_blocks; ++block) {
        uint64_t W[80] = {0};

        // Bước 1: Parse 16 word đầu tiên (1024 bit) từ mảng byte (theo chuẩn Big-Endian)
        for (int t = 0; t < 16; ++t) {
            size_t offset = block * 128 + t * 8;
            W[t] = (static_cast<uint64_t>(padded_data[offset]) << 56) |
                   (static_cast<uint64_t>(padded_data[offset + 1]) << 48) |
                   (static_cast<uint64_t>(padded_data[offset + 2]) << 40) |
                   (static_cast<uint64_t>(padded_data[offset + 3]) << 32) |
                   (static_cast<uint64_t>(padded_data[offset + 4]) << 24) |
                   (static_cast<uint64_t>(padded_data[offset + 5]) << 16) |
                   (static_cast<uint64_t>(padded_data[offset + 6]) << 8) |
                   (static_cast<uint64_t>(padded_data[offset + 7]));
        }

        // Bước 2: Message Schedule - Mở rộng 16 word thành 80 word
        for (int t = 16; t < 80; ++t) {
            W[t] = SmallSigma1(W[t - 2]) + W[t - 7] + SmallSigma0(W[t - 15]) + W[t - 16];
        }

        // Bước 3: Khởi tạo biến làm việc a-h cho block hiện tại
        uint64_t a = H[0], b = H[1], c = H[2], d = H[3];
        uint64_t e = H[4], f = H[5], g = H[6], h = H[7];

        // Bước 4: Chạy 80 vòng lặp (Rounds)
        for (int t = 0; t < 80; ++t) {
            uint64_t T1 = h + BigSigma1(e) + Ch(e, f, g) + K[t] + W[t];
            uint64_t T2 = BigSigma0(a) + Maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }

        // Bước 5: Cập nhật lại giá trị Hash trung gian
        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }

    // Chuyển kết quả ra chuỗi Hex (chữ thường)
    std::stringstream ss;
    for (int i = 0; i < 8; ++i) {
        ss << std::setfill('0') << std::setw(16) << std::hex << H[i];
    }
    return ss.str();
}

} // namespace sha512