#include <zvcr_utils/util/registry_wrapper.hpp>

namespace zvcr {
    void RegistryWrapper::load() {
        air      = mc.blockState("air");
        cave_air = mc.blockState("cave_air");
        void_air = mc.blockState("void_air");

        water = mc.blockType("water");
        lava  = mc.blockType("lava");
    }

    bool RegistryWrapper::isAir(const mc::BlockState state) const {
        return state == air || state == cave_air || state == void_air;
    }

    bool RegistryWrapper::isLiquid(const mc::BlockState state) const {
        return state == water || state == lava;
    }

    auto loadRegistryWrappers(const std::filesystem::path &parentDirectory) -> ErrorMessage {
        return loadRegistryWrappers(parentDirectory, {
            mc::SupportedMinecraftVersion::RELEASE_1_20_4,
            mc::SupportedMinecraftVersion::RELEASE_1_21_4,
        });
    }

    auto loadRegistryWrappers(const std::filesystem::path &parentDirectory, const std::vector<mc::SupportedMinecraftVersion> &mcVersions) -> ErrorMessage {
        TRY(mc::loadRegistries(parentDirectory, mcVersions));
        for (const auto mcVersion : mcVersions) {
            RegistryWrapper registry { getRegistry(mcVersion) };
            registry.load();
            REGISTRY_WRAPPERS.emplace(mcVersion, registry);
        }
        return {};
    }

    auto getRegistryWrapper(const mc::SupportedMinecraftVersion mcVersion) -> const RegistryWrapper& {
        if (!REGISTRY_WRAPPERS.contains(mcVersion))
            throw std::runtime_error("Unsupported protocol version: "
                + std::to_string(mc::PROTOCOL_VERSIONS.at(static_cast<size_t>(mcVersion))));

        return REGISTRY_WRAPPERS.at(mcVersion);
    }

    auto getRegistryWrapper(const uint16_t protocolVersion) -> const RegistryWrapper& {
        for (size_t i = 0; i < mc::PROTOCOL_VERSIONS.size(); i++) {
            if (mc::PROTOCOL_VERSIONS[i] == protocolVersion)
                return getRegistryWrapper(static_cast<mc::SupportedMinecraftVersion>(i));
        }
        throw std::runtime_error("Unsupported protocol version: " + std::to_string(protocolVersion));
    }
}

