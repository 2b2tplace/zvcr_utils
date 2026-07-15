#pragma once

#include <zvcr/region/segment.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <mc_cpp/registry/minecraft.hpp>

#include "mc_cpp/anvil/anvil_region.hpp"

#include "zvcr_utils/types.hpp"

namespace zvcr {

    auto createZVCRSegment(const mc::MinecraftRegistry &registry, DimensionType dimensionType,
                           const mc::NbtFile &chunkNBT, time_t timestamp) -> ResultMessage<std::shared_ptr<Segment>>;

    auto createZVCRRegion(const mc::MinecraftRegistry &registry, DimensionType dimensionType,
                          const mc::anvil::Region &mcRegion, Region &region,
                          const result::Option<time_t> &epochOverride) -> ErrorMessage;
}
