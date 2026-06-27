#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>

namespace zvcr {

    struct StringArgument final : Argument<std::string> {
        explicit StringArgument(ArgumentOptions<std::string> options): Argument(std::move(options)) {}

        explicit StringArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const std::string &value) const -> std::string override;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<std::string>> override;
    };

}