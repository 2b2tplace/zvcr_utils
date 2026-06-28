#pragma once

#include <utility>
#include <zvcr_utils/util/math.hpp>
#include <zvcr_utils/util/lru_cache.hpp>
#include <zvcr/definitions.hpp>
#include <zvcr/io/file_location.hpp>
#include <zvcr/region/segment.hpp>
#include <zvcr/io/serialize/deserialize.hpp>
#include <absl/container/flat_hash_map.h>

namespace zvcr {

    namespace fs = std::filesystem;

    using SegmentID = uint64_t;

    [[nodiscard]]
    auto getSegmentID(int absX, int absZ) -> SegmentID;

    [[nodiscard]]
    auto getAbsoluteSegmentCoordinate(int sc, int rc) -> int;

    [[nodiscard]]
    auto getSegmentID(int sx, int sz, int rx, int rz) -> SegmentID;

    struct SegmentCache {
        uint16_t protocolVersion{};
        time_t epoch{};
        std::shared_ptr<Segment> packed;

        mutable std::array<std::once_flag, MAX_SECTION_COUNT> flagsBlocks;
        mutable std::array<std::once_flag, MAX_SECTION_COUNT> flagsBiomes;

        mutable std::array<result::Option<UnpackedView<SECTION_SIZE_BLOCKS>>, MAX_SECTION_COUNT> unpackedBlocks;
        mutable std::array<result::Option<UnpackedView<SECTION_SIZE_BIOMES>>, MAX_SECTION_COUNT> unpackedBiomes;

        auto blockSection(size_t index) const -> const UnpackedView<SECTION_SIZE_BLOCKS>&;

        auto biomeSection(size_t index) const -> const UnpackedView<SECTION_SIZE_BIOMES>&;
    };

    using SharedSegment = std::shared_ptr<SegmentCache>;

    struct ScanningParameters {
        fs::path parentDirectory;
        DimensionType dimensionType;
        time_t epoch{time(nullptr)};
        size_t threads{std::thread::hardware_concurrency()};
        size_t regionCacheCapacity{16};

        [[nodiscard]]
        auto getLocation(int rx, int rz) const -> RegionLocation;
    };

    class SegmentSupplier {
        using Supplier = std::function<auto() -> SharedSegment>;

        mutable std::once_flag flag;
        mutable result::Option<SharedSegment> segment;
        Supplier supplier;

    public:
        explicit SegmentSupplier(Supplier supplier): supplier(std::move(supplier)) {}

        auto get() const -> SharedSegment;
    };

    class ScanningCache {
    public:
        struct CachedRegion {
            Region region;
            absl::flat_hash_map<SegmentID, SharedSegment> segmentCache;

            auto put(int rx, int rz, time_t epoch) -> void;
        };

        LRUCache<RegionID, CachedRegion> regionCache;
        ScanningParameters parameters;

        explicit ScanningCache(ScanningParameters parameters):
            regionCache(parameters.regionCacheCapacity),
            parameters(std::move(parameters)) {}

        auto load(int rx, int rz) -> std::shared_ptr<CachedRegion>;

        [[nodiscard]]
        auto get(RegionID regionID, SegmentID segmentID) -> SharedSegment;

        [[nodiscard]]
        auto get(SegmentID segmentID) -> SharedSegment;

        [[nodiscard]]
        auto get(int sx, int sz, int rx, int rz) -> SharedSegment;

        [[nodiscard]]
        auto supply(int sx, int sz, int rx, int rz) -> SegmentSupplier;
    };

}
