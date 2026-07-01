#include <zvcr_utils/core/import.hpp>
#include <zvcr/region/paletted_delta_data.hpp>
#include <mc_cpp/anvil/anvil_chunk.hpp>

namespace zvcr {
    auto createZVCRSegment(const mc::MinecraftRegistry &registry, DimensionType dimensionType,
                           const mc::NbtFile &chunkNBT, const time_t timestamp) -> std::shared_ptr<Segment> {
        auto status = chunkNBT.read<std::string>("Status");
        mc::stripMinecraftNamespace(&status);
        if (status != "full") return nullptr;

        const auto chunk = mc::anvil::Chunk::readSections(chunkNBT, registry);
        if (!chunk) return nullptr;

        const auto &properties = dimensionProperties(dimensionType);
        const auto minSectionY = properties.minY / static_cast<int32_t>(SEGMENT_SIDELENGTH_BLOCKS);
        const auto maxSectionY = minSectionY + static_cast<int32_t>(properties.height / SEGMENT_SIDELENGTH_BLOCKS);

        auto segment = std::make_shared<Segment>(dimensionType);
        for (const auto &section : chunk->sections) {
            if (section.yLevel < minSectionY || section.yLevel > maxSectionY) continue;

            const auto i = section.yLevel - minSectionY;
            IGNORE(segment->blockSections.sections[i].insertSnapshot(PackedSnapshot{
                PackedData<SECTION_SIZE_BLOCKS>::pack(section.unpackedBlockData), timestamp
            }));
            IGNORE(segment->biomeSections.sections[i].insertSnapshot(PackedSnapshot{
                PackedData<SECTION_SIZE_BIOMES>::pack(section.unpackedBiomeData), timestamp
            }));
        }
        std::vector<TileEntity> tileEntities;
        for (auto &tileEntityNBT : chunk->tileEntities.values) {
            if (!tileEntityNBT || tileEntityNBT->getType() != mc::NbtType::COMPOUND) continue;

            auto &compound = dynamic_cast<mc::NbtCompound&>(*tileEntityNBT);
            const auto type = registry.tileEntityId(compound.read<std::string>("id"));
            const auto x = compound.read<int32_t>("x");
            const auto y = compound.read<int32_t>("y") - static_cast<int32_t>(SEGMENT_SIDELENGTH_BLOCKS * minSectionY);
            const auto z = compound.read<int32_t>("z");

            compound.entries.erase("id");
            compound.entries.erase("keepPacked");
            compound.entries.erase("x");
            compound.entries.erase("y");
            compound.entries.erase("z");

            std::ostringstream oss;
            compound.writeWithType(oss);
            const auto &nbtStr = oss.str();
            std::vector<uint8_t> nbtBytes{nbtStr.begin(), nbtStr.end()};

            tileEntities.emplace_back(
                static_cast<uint32_t>(type),
                TileEntityPosition{
                    .x = static_cast<uint8_t>(x),
                    .z = static_cast<uint8_t>(z),
                    .y = static_cast<uint16_t>(y)
                },
                nbtBytes
            );
        }
        IGNORE(segment->tileEntities.insertSnapshot(timestamp, tileEntities));
        return segment;
    }

    auto createZVCRRegion(const mc::MinecraftRegistry &registry, const DimensionType dimensionType,
                          const mc::anvil::Region &mcRegion, Region &region,
                          const result::Option<time_t> &epochOverride) -> void {
        for (uint8_t x = 0; x < mc::anvil::REGION_SIDELENGTH_CHUNKS; x++) {
            for (uint8_t z = 0; z < mc::anvil::REGION_SIDELENGTH_CHUNKS; z++) {
                if (!mcRegion.hasChunkViewAt(x, z)) continue;

                const auto &chunkView = mcRegion.chunkViewAt(x, z);
                const auto timestamp = epochOverride.value_or(static_cast<time_t>(chunkView.timestamp));
                const auto chunkNBT = mcRegion.readChunkNbtAt(x, z);
                const auto segment = createZVCRSegment(registry, dimensionType, chunkNBT, timestamp);
                region.set(x, z, segment);
            }
        }
    }
}
