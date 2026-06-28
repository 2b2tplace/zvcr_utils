#pragma once

#include <functional>
#include <zvcr_utils/util/thread_pool.hpp>
#include <zvcr_utils/util/progress.hpp>
#include <zvcr_utils/scanning/scanning_cache.hpp>
#include <zvcr_utils/scanning/region_iterator.hpp>
#include <zvcr/definitions.hpp>

namespace zvcr {

    enum Adjacency {
        NORTHWEST = 0,
        NORTH,
        NORTHEAST,
        WEST,
        THIS,
        EAST,
        SOUTHWEST,
        SOUTH,
        SOUTHEAST
    };

    enum class Direction {
        ABOVE,
        BELOW,
        NORTH,
        SOUTH,
        WEST,
        EAST,
        THIS
    };

    static const auto FACING_FROM_STRING = std::unordered_map<std::string, Direction> {
        {"up", Direction::ABOVE},
        {"down", Direction::BELOW},
        {"north", Direction::NORTH},
        {"south", Direction::SOUTH},
        {"west", Direction::WEST},
        {"east", Direction::EAST}
    };

    using AdjacentSegments = std::array<SegmentSupplier, 9>;

    [[nodiscard]]
    auto getAdjacentIndex(int adjX, int adjZ) -> size_t;

    [[nodiscard]]
    auto getAdjacentIndex(int *localBlockX, int *localBlockZ, uint8_t sidelength) -> size_t;

    [[nodiscard]]
    auto getAdjacentSegment(const AdjacentSegments &adj, int *localBx, int *localBz, uint8_t sidelength) -> SharedSegment;

    [[nodiscard]]
    auto getAdjacentSegmentOpt(const AdjacentSegments &adj, int *localX, int *localZ, uint8_t sidelength) -> SharedSegment;

    auto getBlock(const AdjacentSegments &adj, int localX, uint16_t y, int localZ) -> result::Option<SegmentAtom>;

    auto getBiome(const AdjacentSegments &adj, int localX, uint16_t y, int localZ) -> result::Option<SegmentAtom>;

    constexpr auto offsetX(const Direction direction) -> int32_t {
        switch (direction) {
            case Direction::WEST: return -1;
            case Direction::EAST: return 1;
            default: return 0;
        }
    }

    constexpr auto offsetZ(const Direction direction) -> int32_t {
        switch (direction) {
            case Direction::NORTH: return -1;
            case Direction::SOUTH: return 1;
            default: return 0;
        }
    }

    constexpr auto offsetY(const Direction direction) -> int32_t {
        switch (direction) {
            case Direction::BELOW: return -1;
            case Direction::ABOVE: return 1;
            default: return 0;
        }
    }

    class ScanningContext {
        template<typename... ScanParams>
        using ScanFunction = std::function<void(int sx, int sz, int rx, int rz, ScanParams...)>;

    public:
        using AdjScanFunction = ScanFunction<SharedSegment, const AdjacentSegments&>;
        using SegmentScanFunction = ScanFunction<SharedSegment>;

        ScanningCache cache;

        explicit ScanningContext(const ScanningParameters &parameters):
            cache(parameters) {}

        static auto initializeProgress(const RegionSource &source, Progress &progress) -> void {
            progress.current = 0;
            progress.total = source.size();
        }

        template<typename ScanFn>
        auto scan(const RegionSource &source, const ScanFn &function, Progress *progress = nullptr, const bool async = true) -> void {
            if (async) scanAsync(source, function, progress);
            else scanSync(source, function, progress);
        }

        template<typename ScanFn>
        auto scanAsync(const RegionSource &source, const ScanFn &function, Progress *progress = nullptr) -> void {
            ThreadPool pool{cache.parameters.threads};
            if (progress)
                initializeProgress(source, *progress);

            for (const auto[rx, rz] : source) {
                pool.enqueue([this, rx, rz, function, progress] {
                    scanRegion(rx, rz, function);
                    if (progress)
                        progress->increment();
                });
            }
        }

        template<typename ScanFn>
        auto scanSync(const RegionSource &source, const ScanFn &function, Progress *progress = nullptr) -> void {
            if (progress)
                initializeProgress(source, *progress);

            for (const auto[rx, rz] : source)
                scanRegion(rx, rz, function);
        }

        template<typename ScanFn>
        auto scanRegion(const int rx, const int rz, const ScanFn &function) -> void {
            const auto cachedRegion = cache.load(rx, rz);

            for (int sx = 0; sx < REGION_SIDELENGTH_SEGMENTS; sx++) {
                for (int sz = 0; sz < REGION_SIDELENGTH_SEGMENTS; sz++) {
                    applyScanFunction(sx, sz, rx, rz, function);
                }
            }
        }

    private:
        auto applyScanFunction(int sx, int sz, int rx, int rz, const SegmentScanFunction &scanFunction) -> void;
        auto applyScanFunction(int sx, int sz, int rx, int rz, const AdjScanFunction &scanFunction) -> void;
    };

}
