#include <zvcr_utils/scanning/scanning_context.hpp>
#include <zvcr_utils/util/math.hpp>

namespace zvcr {
    auto getAdjacentIndex(const int adjX, const int adjZ) -> size_t {
        const auto adjIndexX = static_cast<size_t>(adjX < 0 ? 0 : adjX > 0 ? 2 : 1);
        const auto adjIndexZ = static_cast<size_t>(adjZ < 0 ? 0 : adjZ > 0 ? 2 : 1);

        return 3 * adjIndexZ + adjIndexX;
    }

    auto getAdjacentIndex(int *localBlockX, int *localBlockZ, const uint8_t sidelength) -> size_t {
        const auto adjIndexX = static_cast<size_t>(*localBlockX < 0 ? 0 : *localBlockX > sidelength - 1 ? 2 : 1);
        const auto adjIndexZ = static_cast<size_t>(*localBlockZ < 0 ? 0 : *localBlockZ > sidelength - 1 ? 2 : 1);

        *localBlockX = mod(*localBlockX, sidelength);
        *localBlockZ = mod(*localBlockZ, sidelength);

        return 3 * adjIndexZ + adjIndexX;
    }

    auto getAdjacentSegment(const AdjacentSegments &adj, int *localBx, int *localBz, const uint8_t sidelength) -> SharedSegment {
        return adj[getAdjacentIndex(localBx, localBz, sidelength)].get();
    }

    auto getAdjacentSegmentOpt(const AdjacentSegments &adj, int *localX, int *localZ, const uint8_t sidelength) -> SharedSegment {
        if (*localX < -sidelength || *localX >= 2 * sidelength
            || *localZ < -sidelength || *localZ >= 2 * sidelength)
            return nullptr;

        return getAdjacentSegment(adj, localX, localZ, sidelength);
    }

    auto getBlock(const AdjacentSegments &adj, int localX, const uint16_t y, int localZ) -> result::Option<SegmentAtom> {
        const auto &segment = getAdjacentSegmentOpt(adj, &localX, &localZ, SEGMENT_SIDELENGTH_BLOCKS);
        if (!segment) return result::None;

        return segment->blockSection(y / SEGMENT_SIDELENGTH_BLOCKS)
                .voxel(localX, y % SEGMENT_SIDELENGTH_BLOCKS, localZ);
    }

    auto getBiome(const AdjacentSegments &adj, int localX, const uint16_t y, int localZ) -> result::Option<SegmentAtom> {
        const auto &segment = getAdjacentSegmentOpt(adj, &localX, &localZ, SEGMENT_SIDELENGTH_BIOMES);
        if (!segment) return result::None;

        return segment->biomeSection(y / SEGMENT_SIDELENGTH_BIOMES)
                .voxel(localX, y % SEGMENT_SIDELENGTH_BIOMES, localZ);
    }

    auto ScanningContext::applyScanFunction(const int sx, const int sz, const int rx, const int rz,
                                            const SegmentScanFunction &scanFunction) -> void {
        const auto segment = cache.get(sx, sz, rx, rz);
        if (!segment) return;

        scanFunction(sx, sz, rx, rz, segment);
    }

    auto ScanningContext::applyScanFunction(const int sx, const int sz, const int rx, const int rz,
                                            const AdjScanFunction &scanFunction) -> void {
        auto segment = cache.get(sx, sz, rx, rz);
        if (!segment) return;

        const AdjacentSegments adj = {
            cache.supply(sx - 1, sz - 1, rx, rz),
            cache.supply(sx,     sz - 1, rx, rz),
            cache.supply(sx + 1, sz - 1, rx, rz),
            cache.supply(sx - 1, sz,     rx, rz),
            SegmentSupplier{[segment] { return segment; }},
            cache.supply(sx + 1, sz,     rx, rz),
            cache.supply(sx - 1, sz + 1, rx, rz),
            cache.supply(sx,     sz + 1, rx, rz),
            cache.supply(sx + 1, sz + 1, rx, rz),
        };
        scanFunction(sx, sz, rx, rz, segment, adj);
    }
}
