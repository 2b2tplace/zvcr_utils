#pragma once

#include <algorithm>
#include <zvcr_utils/cli/arguments/argument_base.hpp>

namespace zvcr {

    struct FlagArgument final : Argument<bool> {
        explicit FlagArgument(ArgumentOptions<bool> options): Argument(std::move(options)) {}

        explicit FlagArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const bool &value) const -> std::string override;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<bool>> override;

        [[nodiscard]]
        static auto create(const std::string &name) -> FlagArgument;
    };

    auto backupExistingArgument() -> FlagArgument;

}