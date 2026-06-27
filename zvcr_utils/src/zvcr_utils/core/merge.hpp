#pragma once

#include <memory>
#include <vector>
#include <absl/container/btree_map.h>
#include <zvcr/region/segment_info.hpp>
#include <zvcr/region/paletted_delta_data.hpp>
#include <zvcr/io/file_type.hpp>

namespace zvcr {

    template<size_t snapshotLength>
    auto unpackAllSegmentSnapshots(const PackedDeltaData<snapshotLength> &deltaData,
                                   absl::btree_map<time_t, UnpackedData<snapshotLength>> &snapshots) -> void {
        const auto &latestPacked = deltaData.latestSnapshot();
        if (!latestPacked) return;

        auto snapshot = latestPacked->get().data.unpack();
        snapshots[latestPacked->get().timestamp] = snapshot;

        bool first = true;
        for (const auto& [sectionData, deltaTimestamp] : deltaData.reverseDeltas) {
            if (first) {
                first = false;
                continue;
            }
            const auto unpacked = sectionData.unpack();
            for (size_t j = 0; j < snapshotLength; ++j) {
                if (const auto state = unpacked[j]; state != STATE_UNCHANGED)
                    snapshot[j] = state;
            }
            snapshots[deltaTimestamp] = snapshot;
        }
    }

    template<size_t snapshotLength>
    auto insertAllSegmentSnapshots(const absl::btree_map<time_t, UnpackedData<snapshotLength>> &unpackedSnapshots,
                                   PackedDeltaData<snapshotLength> &mergedData) -> bool {
        bool success{};
        for (const auto &[timestamp, snapshot] : unpackedSnapshots) {
            const auto blockSnapshotResult = mergedData.insertSnapshot(PackedSnapshot{
                .data = PackedData<snapshotLength>::pack(snapshot),
                .timestamp = timestamp
            });
            if (blockSnapshotResult)
                success = true;
            else if (blockSnapshotResult.error() != DeltaInsertionStatus::NO_CHANGES_MADE) {
                std::cerr << "Failed insertion: " << (int) blockSnapshotResult.error() << '\n';
            }
        }
        return success;
}

    auto mergeSegments(const std::vector<std::shared_ptr<Segment>> &sources, DimensionType dimensionType) -> std::shared_ptr<Segment>;

    auto mergeSegments(const std::vector<Region> &sources, Region &out,
                       uint8_t x, uint8_t z, DimensionType dimensionType) -> bool;

}
