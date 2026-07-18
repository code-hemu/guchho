#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace guchho::compat {

struct Semver {
    std::vector<int> parts;
    std::string pre_release;
};

Semver parse_semver(const std::string &text);
int compare(const Semver &a, const Semver &b);

enum class JSFeature : uint64_t {
    None = 0,
    Arrow = 1 << 0,
    Const = 1 << 1,
    Let = 1 << 2,
    AsyncAwait = 1 << 3,
    OptionalChaining = 1 << 4,
    NullishCoalescing = 1 << 5,
    Class = 1 << 6,
    Spread = 1 << 7,
    Destructuring = 1 << 8,
    RestArgs = 1 << 9,
    DefaultArg = 1 << 10,
    Generator = 1 << 11,
    ForOf = 1 << 12,
    Template = 1 << 13,
    Exponent = 1 << 14,
    BigInt = 1 << 15,
    DynamicImport = 1 << 16,
};

enum class CSSFeature : uint64_t {
    None = 0,
    Nesting = 1 << 0,
    CustomProperties = 1 << 1,
    ColorFunction = 1 << 2,
};

}
