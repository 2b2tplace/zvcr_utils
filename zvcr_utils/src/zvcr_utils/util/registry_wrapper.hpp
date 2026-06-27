#pragma once

#include <absl/container/flat_hash_map.h>
#include <mc_cpp/registry/minecraft.hpp>
#include <zvcr_utils/types.hpp>

namespace zvcr {

    struct RegistryWrapper {
        const mc::MinecraftRegistry &mc;

        mc::BlockState air;
        mc::BlockState cave_air;
        mc::BlockState void_air;

        mc::BlockType water;
        mc::BlockType lava;

        void load();

        [[nodiscard]]
        bool isAir(mc::BlockState state) const;

        [[nodiscard]]
        bool isLiquid(mc::BlockState state) const;
    };

    inline auto REGISTRY_WRAPPERS = absl::flat_hash_map<mc::SupportedMinecraftVersion, RegistryWrapper>{};

    [[nodiscard]]
    auto loadRegistryWrappers(const std::filesystem::path &parentDirectory) -> ErrorMessage;

    [[nodiscard]]
    auto loadRegistryWrappers(const std::filesystem::path &parentDirectory,
                              const std::vector<mc::SupportedMinecraftVersion> &mcVersions) -> ErrorMessage;

    [[nodiscard]]
    auto getRegistryWrapper(mc::SupportedMinecraftVersion mcVersion) -> const RegistryWrapper&;

    [[nodiscard]]
    auto getRegistryWrapper(uint16_t protocolVersion) -> const RegistryWrapper&;
}
