#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <filesystem>

namespace zvcr {

    namespace fs = std::filesystem;

    inline constexpr auto REGISTRIES_ENV_VAR = "ZVCR_CLI_MINECRAFT_REGISTRIES_PATH";

    [[nodiscard]]
    auto registriesEnvVar() -> result::Option<std::string>;

    struct FilepathArgument final : Argument<fs::path> {
        explicit FilepathArgument(ArgumentOptions<fs::path> options): Argument(std::move(options)) {}

        explicit FilepathArgument(const Argument &other): Argument(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override;

        [[nodiscard]]
        auto stringify(const fs::path &value) const -> std::string override;

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<fs::path>> override;

        static auto registries() -> FilepathArgument;
    };

}