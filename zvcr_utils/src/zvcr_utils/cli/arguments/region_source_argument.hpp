#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <zvcr_utils/scanning/region_iterator.hpp>

namespace zvcr {

    struct RegionSourceArgument final : Argument<Rect> {
        explicit RegionSourceArgument(ArgumentOptions<Rect> options): Argument(std::move(options)) {}

        explicit RegionSourceArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const Rect &value) const -> std::string override;

        [[nodiscard]]
        static auto parseCoordinates(const std::string &pairStr) -> ResultMessage<RegionCoordinates>;

        [[nodiscard]]
        static auto parseRect(const std::string &rectStr) -> ResultMessage<Rect>;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<Rect>> override;

        [[nodiscard]]
        auto parseAsSource(const ArgumentParser &parser, const std::vector<fs::path> &rootDirs,
                           std::string_view extension) const -> ResultMessage<std::unique_ptr<RegionSource>>;

        [[nodiscard]]
        static auto create() -> RegionSourceArgument;
    };

}