//
// Created by PC on 2026/6/28.
//

#ifndef CC_BITSET_HASH_H
#define CC_BITSET_HASH_H

#include <bitset>
#include <cstdint>
#include <cstring>
#include <unordered_map>

namespace cc {

template <size_t N>
struct BitsetHash {
    static_assert(N % 64 == 0,
            "BitsetHash requires N to be a multiple of 64 for optimal 64-bit block hashing.");
    static_assert(N > 0, "BitsetHash requires N to be greater than 0.");

    static constexpr bool kHasPaddingBits = sizeof(std::bitset<N>) != N / 8;

    size_t operator()(const std::bitset<N>& bs) const {
        if (kHasPaddingBits) {
            // 改用字符串哈希
            return std::hash<std::string>{}(bs.to_string());
        }
        constexpr size_t kNumBlocks = N / 64;

        // 转成 uint64数组
        alignas(uint64_t) uint64_t blocks[kNumBlocks];
        std::memcpy(blocks, &bs, sizeof(bs));
        // 混合哈希
        size_t hash = 0;
        for (size_t i = 0; i < kNumBlocks; ++i) {
            size_t block_hash = std::hash<uint64_t>{}(blocks[i]);
            hash ^= block_hash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

template <typename V, size_t N>
using BitsetMap = std::unordered_map<std::bitset<N>, V, BitsetHash<N>>;

}  // namespace cc

#endif  //CC_BITSET_HASH_H
