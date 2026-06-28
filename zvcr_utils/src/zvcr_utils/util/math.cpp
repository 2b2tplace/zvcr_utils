#include <zvcr_utils/util/math.hpp>
#include <cmath>

namespace zvcr {
    auto floorDiv(const int a, const int b) -> int {
        auto q = a / b;
        if (const auto r = a % b; r != 0 && r > 0 != b > 0) --q;
        return q;
    }

    int roundToEven(const double x) {
        return static_cast<int>(std::floor(x / 2.0) * 2.0);
    }

    auto mod(const int a, const int b) -> int {
        const auto ret = a % b;
        return ret >= 0 ? ret : ret + b;
    }

    void hashCombine(size_t &seed, const size_t value) {
        seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    }
}
