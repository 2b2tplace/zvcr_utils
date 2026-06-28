#include <zvcr_utils/scanning/scanning_cache.hpp>

namespace zvcr {
    auto getSegmentID(const int absX, const int absZ) -> SegmentID {
        return static_cast<int64_t>(absZ) << 32 | static_cast<int64_t>(absX) & 0xFFFFFFFF;
    }

    auto getAbsoluteSegmentCoordinate(const int sc, const int rc) -> int {
        return rc * static_cast<int>(REGION_SIDELENGTH_SEGMENTS) + sc;
    }

    auto getSegmentID(const int sx, const int sz, const int rx, const int rz) -> SegmentID {
        return getSegmentID(getAbsoluteSegmentCoordinate(sx, rx), getAbsoluteSegmentCoordinate(sz, rz));
    }

    auto SegmentCache::blockSection(const size_t index) const -> const UnpackedView<SECTION_SIZE_BLOCKS>& {
        std::call_once(flagsBlocks.at(index), [&] {
            const auto snapshot = packed->blockSections.sections.at(index).snapshotFrom(epoch);
            if (!snapshot) return;

            unpackedBlocks.at(index).emplace(static_cast<uint8_t>(SEGMENT_SIDELENGTH_BLOCKS), *snapshot);
        });
        return *unpackedBlocks.at(index);
    }

    auto SegmentCache::biomeSection(const size_t index) const -> const UnpackedView<SECTION_SIZE_BIOMES>& {
        std::call_once(flagsBiomes.at(index), [&] {
            const auto snapshot = packed->biomeSections.sections.at(index).snapshotFrom(epoch);
            if (!snapshot) return;

            unpackedBiomes.at(index).emplace(static_cast<uint8_t>(SEGMENT_SIDELENGTH_BIOMES), *snapshot);
        });
        return *unpackedBiomes.at(index);
    }

    auto ScanningParameters::getLocation(const int rx, const int rz) const -> RegionLocation {
        return RegionLocation { rx, rz, dimensionType };
    }

    auto SegmentSupplier::get() const -> SharedSegment {
        std::call_once(flag, [&] {
            segment.emplace(supplier());
        });
        return *segment;
    }

    auto ScanningCache::CachedRegion::put(const int rx, const int rz, const time_t epoch) -> void {
        for (uint8_t sz = 0; sz < REGION_SIDELENGTH_SEGMENTS; sz++) {
            for (uint8_t sx = 0; sx < REGION_SIDELENGTH_SEGMENTS; sx++) {
                const auto segmentID = getSegmentID(sx, sz, rx, rz);
                if (segmentCache.contains(segmentID)) continue;

                const auto &segment = region.get(sx, sz);
                if (!segment) continue;

                const auto protocolVersion = region.protocolVersion;
                segmentCache[segmentID] = std::make_shared<SegmentCache>(
                    protocolVersion,
                    epoch,
                    segment
                );
            }
        }
    }

    auto ScanningCache::load(const int rx, const int rz) -> std::shared_ptr<CachedRegion> {
        const auto location = parameters.getLocation(rx, rz);
        const auto regionID = location.toRegionID();
        auto cached = regionCache.get(regionID);

        if (cached != nullptr) return cached;

        auto readResult = readFileAt(parameters.parentDirectory, location);
        if (!readResult.has_value()) return nullptr;

        auto region = std::make_shared<CachedRegion>(std::move(readResult->region));
        region->put(rx, rz, parameters.epoch);

        regionCache.put(regionID, region);
        return region;
    }

    auto ScanningCache::get(const RegionID regionID, const SegmentID segmentID) -> SharedSegment {
        const auto cachedRegion = regionCache.get(regionID);
        if (cachedRegion == nullptr) return nullptr;
        if (cachedRegion->segmentCache.contains(segmentID)) return cachedRegion->segmentCache.at(segmentID);

        return nullptr;
    }

    auto ScanningCache::get(const SegmentID segmentID) -> SharedSegment {
        const auto rx = floorDiv(static_cast<int>(segmentID & 0xFFFFFFFF), REGION_SIDELENGTH_SEGMENTS);
        const auto rz = floorDiv(static_cast<int>(segmentID >> 32), REGION_SIDELENGTH_SEGMENTS);
        const auto regionID = parameters.getLocation(rx, rz).toRegionID();

        if (const auto segment = get(regionID, segmentID); segment != nullptr)
            return segment;

        if (!load(rx, rz)) return nullptr;

        return get(regionID, segmentID);
    }

    auto ScanningCache::get(const int sx, const int sz, const int rx, const int rz) -> SharedSegment {
        return get(getSegmentID(sx, sz, rx, rz));
    }

    auto ScanningCache::supply(const int sx, const int sz, const int rx, const int rz) -> SegmentSupplier {
        return SegmentSupplier{[=, this] {
            return get(sx, sz, rx, rz);
        }};
    }
}
