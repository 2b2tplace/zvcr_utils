#include <zvcr_utils/core/export.hpp>

namespace zvcr {

    auto createAnvilChunk(const mc::MinecraftRegistry &registry, const DimensionType dimensionType,
                          const mc::anvil::Region &region, const mc::anvil::Pos &absoluteChunkPos,
                          const Segment &segment, const time_t epoch) -> result::Option<mc::NbtFile> {
        const auto &blockSnapshot = segment.blockSections.snapshotFrom(epoch);
        const auto &biomeSnapshot = segment.biomeSections.snapshotFrom(epoch);
        if (!blockSnapshot || !biomeSnapshot) return {};

        const auto &dimensionProperties = mc::getDimensionProperties(static_cast<mc::DimensionType>(dimensionType));
        const auto sectionCount = dimensionProperties.height / mc::anvil::SECTION_SIDELENGTH_BLOCKS;
        const auto minChunkY = dimensionProperties.minY / mc::anvil::SECTION_SIDELENGTH_BLOCKS;

        mc::anvil::Chunk chunk;
        chunk.sections.reserve(sectionCount);

        for (int32_t i = 0; i < sectionCount; i++) {
            mc::anvil::ChunkSection section{registry, static_cast<int8_t>(i + minChunkY)};
            section.unpackedBlockData = blockSnapshot->at(i);
            section.unpackedBiomeData = biomeSnapshot->at(i);

            chunk.sections.push_back(section);
        }
        if (const auto &tileEntities = segment.tileEntities.snapshotFrom(epoch)) {
            for (const auto &[tileEntityPos, tileEntity] : *tileEntities) {
                mc::NbtCompound tileEntityNBT;
                tileEntityNBT.put("id", registry.tileEntityName(tileEntity.type));
                tileEntityNBT.put("keepPacked", false);
                tileEntityNBT.put<int32_t>("x", tileEntityPos.x);
                tileEntityNBT.put<int32_t>("y", SEGMENT_SIDELENGTH_BLOCKS * minChunkY + static_cast<int32_t>(tileEntityPos.y));
                tileEntityNBT.put<int32_t>("z", tileEntityPos.z);

                std::istringstream iss(std::string{tileEntity.nbt.begin(), tileEntity.nbt.end()});
                const auto &addNBT = mc::read(iss);

                if (addNBT && addNBT->getType() == mc::NbtType::COMPOUND) {
                    const auto &addCompound = mc::getAsTag<mc::NbtCompound>(*addNBT);
                    for (const auto &[key, value] : addCompound.entries) {
                        tileEntityNBT.entries[key] = value->clone();
                    }
                }
                chunk.tileEntities.add(tileEntityNBT);
            }
        }
        auto chunkNBT = region.createChunk(absoluteChunkPos, minChunkY, registry);
        chunk.writeSections(chunkNBT, registry);
        return chunkNBT;
    }

    auto createAnvilRegion(const fs::path &parentDirectoryOut, DimensionType dimensionType,
                           const mc::anvil::Pos &regionPos, const Region &region, const time_t epoch) -> mc::anvil::Region {
        const auto &registry = getRegistryWrapper(region.protocolVersion);
        mc::anvil::Region mcRegion{parentDirectoryOut, static_cast<mc::DimensionType>(dimensionType), regionPos};
        for (uint8_t x = 0; x < REGION_SIDELENGTH_SEGMENTS; x++) {
            for (uint8_t z = 0; z < REGION_SIDELENGTH_SEGMENTS; z++) {
                const auto &segment = region.get(x, z);
                if (!segment) continue;

                const auto chunkPos = mcRegion.absoluteChunkPos({x, z});
                const auto chunkNBT = createAnvilChunk(registry.mc, dimensionType, mcRegion, chunkPos, *segment, epoch);
                if (!chunkNBT) continue;

                absl::flat_hash_set<time_t> availableTimestamps;
                availableTimestamps.reserve(segment->tileEntities.reverseDeltas.size());
                for (const auto &[timestamp, _] : segment->tileEntities.reverseDeltas)
                    availableTimestamps.emplace(timestamp);

                for (size_t sectionIndex = 0; sectionIndex < segment->sectionCount; sectionIndex++) {
                    for (const auto &[_, timestamp] : segment->blockSections.sections[sectionIndex].reverseDeltas)
                        availableTimestamps.emplace(timestamp);

                    for (const auto &[_, timestamp] : segment->biomeSections.sections[sectionIndex].reverseDeltas)
                        availableTimestamps.emplace(timestamp);
                }
                mcRegion.writeChunkNbtAt(chunkPos, *chunkNBT);
                mcRegion.chunkViewAt(chunkPos).timestamp = findNearestTimestamp(availableTimestamps, [](const auto t) { return t; }, epoch);
            }
        }
        return mcRegion;
    }
}
