#pragma once

#include <cstdint>
#include <string>

namespace guchho::xxhash {

class Digest {
public:
    Digest();

    void write(const void *data, size_t size);
    void write(const std::string &data);
    uint64_t sum64() const;
    void reset();

private:
    uint64_t v1_;
    uint64_t v2_;
    uint64_t v3_;
    uint64_t v4_;
    uint64_t total_;
    uint8_t mem_[32];
    size_t mem_size_;

    static constexpr uint64_t prime1 = 11400714785074694791ULL;
    static constexpr uint64_t prime2 = 14029467366897019727ULL;
    static constexpr uint64_t prime3 = 1609587929392839161ULL;
    static constexpr uint64_t prime4 = 9650029242287828579ULL;
    static constexpr uint64_t prime5 = 2870177450012600261ULL;

    void process_round(uint64_t &v, uint64_t input) const;
    void process_block(const uint64_t *block);
};

uint64_t hash(const void *data, size_t size);
uint64_t hash(const std::string &data);

}
