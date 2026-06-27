#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>

namespace zvcr {

    using Range = std::pair<size_t, size_t>;

    struct RangeArgument final : Argument<Range> {
        explicit RangeArgument(ArgumentOptions<Range> options): Argument(std::move(options)) {}

        explicit RangeArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const Range &value) const -> std::string override;

        [[nodiscard]]
        static auto parseRange(const std::string &rangeStr) -> ResultMessage<Range>;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<Range>> override;
    };

}