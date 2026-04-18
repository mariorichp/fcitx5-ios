// iOS 16.1 libc++ compatibility shim.
//
// Xcode 26 SDK ships a libc++ that emits calls to std::__1::__hash_memory
// (introduced in iOS 17). On iOS 16.1 the system /usr/lib/libc++.1.dylib
// does not export this symbol. With -Wl,-weak-lc++ the import resolves to
// NULL and the first std::unordered_map<std::string, ...> insert — which
// happens during static initializers (e.g. fcitx addon registration) —
// jumps to address 0 and the app crashes before main().
//
// Providing a strong definition in a static library linked into every
// binary pre-empts the dylib import: ld64 prefers a local archive symbol
// over a weakly-imported dylib symbol, so the call goes to this function
// and no runtime lookup against libc++ is needed.
//
// Implementation is Murmur2 64-bit — what pre-__hash_memory libc++ used
// internally. Hash quality is irrelevant for correctness; only stable
// per-process distribution matters (unordered_map works with any hash).

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace std {
inline namespace __1 {

__attribute__((visibility("default")))
size_t __hash_memory(const void* key, size_t len) noexcept {
    constexpr uint64_t m = 0xc6a4a7935bd1e995ULL;
    constexpr int r = 47;
    uint64_t h = len * m;
    const uint8_t* data = static_cast<const uint8_t*>(key);
    while (len >= 8) {
        uint64_t k;
        std::memcpy(&k, data, 8);
        k *= m;
        k ^= k >> r;
        k *= m;
        h ^= k;
        h *= m;
        data += 8;
        len -= 8;
    }
    switch (len) {
        case 7: h ^= uint64_t(data[6]) << 48; [[fallthrough]];
        case 6: h ^= uint64_t(data[5]) << 40; [[fallthrough]];
        case 5: h ^= uint64_t(data[4]) << 32; [[fallthrough]];
        case 4: h ^= uint64_t(data[3]) << 24; [[fallthrough]];
        case 3: h ^= uint64_t(data[2]) << 16; [[fallthrough]];
        case 2: h ^= uint64_t(data[1]) << 8;  [[fallthrough]];
        case 1: h ^= uint64_t(data[0]);
                h *= m;
    }
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return static_cast<size_t>(h);
}

} // namespace __1
} // namespace std
