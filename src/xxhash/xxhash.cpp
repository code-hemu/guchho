#include "xxhash/xxhash.hpp"
#include <cstring>

namespace guchho::xxhash {

Digest::Digest() {
    reset();
}

void Digest::reset() {
    v1_ = prime1 + prime2;
    v2_ = prime2;
    v3_ = 0;
    v4_ = prime1 - prime3;
    total_ = 0;
    mem_size_ = 0;
}

void Digest::process_round(uint64_t &v, uint64_t input) const {
    v += input * prime2;
    v = (v << 31) | (v >> 33);
    v *= prime1;
}

void Digest::process_block(const uint64_t *block) {
    process_round(v1_, block[0]);
    process_round(v2_, block[1]);
    process_round(v3_, block[2]);
    process_round(v4_, block[3]);
}

void Digest::write(const void *data, size_t size) {
    auto *bytes = static_cast<const uint8_t *>(data);
    total_ += size;

    if (mem_size_ + size < 32) {
        std::memcpy(mem_ + mem_size_, bytes, size);
        mem_size_ += size;
        return;
    }

    if (mem_size_ > 0) {
        size_t fill = 32 - mem_size_;
        std::memcpy(mem_ + mem_size_, bytes, fill);
        process_block(reinterpret_cast<const uint64_t *>(mem_));
        bytes += fill;
        size -= fill;
        mem_size_ = 0;
    }

    while (size >= 32) {
        process_block(reinterpret_cast<const uint64_t *>(bytes));
        bytes += 32;
        size -= 32;
    }

    if (size > 0) {
        std::memcpy(mem_, bytes, size);
        mem_size_ = size;
    }
}

void Digest::write(const std::string &data) {
    write(data.data(), data.size());
}

uint64_t Digest::sum64() const {
    uint64_t h = total_;

    if (total_ >= 32) {
        h = (v1_ ^ (v2_ * prime5)) * prime1;
        h = (h + (v3_ * prime4)) * prime1;
        h = (h + v4_);
    }

    auto *p = mem_;
    auto *end = mem_ + mem_size_;

    while (p + 8 <= end) {
        uint64_t k;
        std::memcpy(&k, p, 8);
        k *= prime2;
        k = (k << 31) | (k >> 33);
        k *= prime1;
        h ^= k;
        h = (h << 27) | (h >> 37);
        h = h * prime1 + prime4;
        p += 8;
    }

    if (p + 4 <= end) {
        uint32_t k;
        std::memcpy(&k, p, 4);
        h ^= uint64_t(k) * prime1;
        h = (h << 23) | (h >> 41);
        h *= prime2;
        h += prime3;
        p += 4;
    }

    while (p < end) {
        h ^= uint64_t(*p) * prime5;
        h = (h << 11) | (h >> 53);
        h *= prime1;
        p++;
    }

    h ^= h >> 33;
    h *= prime2;
    h ^= h >> 29;
    h *= prime3;
    h ^= h >> 32;

    return h;
}

uint64_t hash(const void *data, size_t size) {
    Digest d;
    d.write(data, size);
    return d.sum64();
}

uint64_t hash(const std::string &data) {
    return hash(data.data(), data.size());
}

}
