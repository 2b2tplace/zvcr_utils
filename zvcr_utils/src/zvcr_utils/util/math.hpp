#pragma once

#include <cstddef>
#include <string>

namespace zvcr {

    [[nodiscard]]
    auto floorDiv(int a, int b) -> int;

    [[nodiscard]]
    int roundToEven(double x);

    [[nodiscard]]
    auto mod(int a, int b) -> int;

    void hashCombine(size_t &seed, size_t value);

    template<typename T, typename... Ts>
    size_t hash(Ts... values) {
        size_t seed = 0;
        std::hash<T> hasher;
        (hashCombine(seed, hasher(values)), ...);
        return seed;
    }

}
