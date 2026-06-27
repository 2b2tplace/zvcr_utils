#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <mc_cpp/anvil/level_dat/position_rotation.hpp>

namespace zvcr {

    struct Vec3dArgument final : Argument<mc::Vec3d> {
        explicit Vec3dArgument(ArgumentOptions<mc::Vec3d> options): Argument(std::move(options)) {}

        explicit Vec3dArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const mc::Vec3d &value) const -> std::string override;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<mc::Vec3d>> override;
    };

}
