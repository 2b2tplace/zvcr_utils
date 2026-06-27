#pragma once

#include <utility>
#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <zvcr/dimension.hpp>

namespace zvcr {

    struct DimensionArgument final : Argument<DimensionType> {
        explicit DimensionArgument(ArgumentOptions<DimensionType> options): Argument(std::move(options)) {}

        explicit DimensionArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const DimensionType &value) const -> std::string override;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<DimensionType>> override;

        [[nodiscard]]
        static auto create() -> DimensionArgument;
    };

}
