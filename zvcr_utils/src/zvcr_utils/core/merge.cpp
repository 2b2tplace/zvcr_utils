#include <zvcr_utils/core/merge.hpp>

namespace zvcr {

    auto mergeSegments(const std::vector<std::shared_ptr<Segment>> &sources, const DimensionType dimensionType) -> std::shared_ptr<Segment> {
        if (sources.empty()) return nullptr;
        if (sources.size() == 1) return sources[0];

        const auto mergedSegment = std::make_shared<Segment>(dimensionType);
        for (size_t i = 0; i < mergedSegment->sectionCount; i++) {
            auto &mergedBlockSection = mergedSegment->blockSections.sections[i];
            auto &mergedBiomeSection = mergedSegment->biomeSections.sections[i];

            absl::btree_map<time_t, UnpackedData<SECTION_SIZE_BLOCKS>> unpackedBlockSectionSnapshots;
            absl::btree_map<time_t, UnpackedData<SECTION_SIZE_BIOMES>> unpackedBiomeSectionSnapshots;

            for (const auto &segment : sources) {
                const auto &blockSection = segment->blockSections.sections[i];
                const auto &biomeSection = segment->biomeSections.sections[i];

                unpackAllSegmentSnapshots(blockSection, unpackedBlockSectionSnapshots);
                unpackAllSegmentSnapshots(biomeSection, unpackedBiomeSectionSnapshots);
            }
            IGNORE(insertAllSegmentSnapshots(unpackedBlockSectionSnapshots, mergedBlockSection));
            IGNORE(insertAllSegmentSnapshots(unpackedBiomeSectionSnapshots, mergedBiomeSection));
        }
        absl::btree_map<time_t, TileEntityList> unpackedTileEntityListSnapshots;
        for (const auto &segment : sources) {
            const auto &tileEntities = segment->tileEntities;
            const auto latestOpt = tileEntities.latestSnapshot();
            if (!latestOpt) continue;

            const auto &latest = latestOpt->get();
            TileEntityList snapshot;
            snapshot.reserve(latest.deltas.size());
            for (const auto &[pos, delta] : latest.deltas) {
                if (std::holds_alternative<TileEntity>(delta))
                    snapshot[pos] = std::get<TileEntity>(delta);
            }
            unpackedTileEntityListSnapshots.emplace(latest.timestamp, snapshot);

            bool first = true;
            for (const auto &[deltaTimestamp, deltas] : tileEntities.reverseDeltas) {
                if (first) {
                    first = false;
                    continue;
                }
                for (const auto &[pos, delta] : deltas) {
                    if (std::holds_alternative<TileEntity>(delta)) {
                        snapshot[pos] = std::get<TileEntity>(delta);
                    } else {
                        snapshot.erase(pos);
                    }
                }
                unpackedTileEntityListSnapshots.emplace(deltaTimestamp, snapshot);
            }
        }
        for (const auto &[timestamp, tileEntityList] : unpackedTileEntityListSnapshots) {
            IGNORE(mergedSegment->tileEntities.insertSnapshot(timestamp, tileEntityList | std::views::values));
        }
        absl::btree_map<time_t, SegmentStateType> segmentStateSnapshots;
        for (const auto &segment : sources) {
            for (const auto &[state, timestamp] : segment->info.segmentStates) {
                segmentStateSnapshots[timestamp] = state;
            }
        }
        for (const auto &[timestamp, state] : segmentStateSnapshots) {
            IGNORE(mergedSegment->info.insertSnapshot(zvcr::SegmentState{.type = state, .timestamp = timestamp}));
        }
        return mergedSegment;
    }

    auto mergeSegments(const std::vector<Region> &sources, Region &out,
                       const uint8_t x, const uint8_t z, const DimensionType dimensionType) -> bool {
        std::vector<std::shared_ptr<Segment>> segments;
        segments.reserve(sources.size());

        for (const auto &region : sources) {
            const auto &segment = region.get(x, z);
            if (!segment) continue;

            segments.push_back(segment);
        }
        const auto segment = mergeSegments(segments, dimensionType);
        out.set(x, z, segment);
        return segment != nullptr;
    }

}