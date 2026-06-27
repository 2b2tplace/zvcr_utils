#pragma once

#include <zvcr_utils/util/registry_wrapper.hpp>
#include <zvcr/region/segment.hpp>
#include <mc_cpp/anvil/anvil_region.hpp>
#include <mc_cpp/registry/minecraft.hpp>

namespace zvcr {

    namespace fs = std::filesystem;

    auto createAnvilChunk(const mc::MinecraftRegistry &registry, DimensionType dimensionType,
                          const mc::anvil::Region &region, const mc::anvil::Pos &absoluteChunkPos,
                          const Segment &segment, time_t epoch) -> result::Option<mc::NbtFile>;

    auto createAnvilRegion(const fs::path &parentDirectoryOut, DimensionType dimensionType,
                           const mc::anvil::Pos &regionPos,
                           const Region &region, time_t epoch) -> mc::anvil::Region;
}
