#include <zvcr_utils/cli/arguments/filepath_argument.hpp>

namespace zvcr {

    auto registriesEnvVar() -> result::Option<std::string> {
        const auto value = std::getenv(REGISTRIES_ENV_VAR);
        if (!value) return result::None;

        return std::string{value};
    }

    auto FilepathArgument::mock() const -> std::string {
        return fmt::format("='/path/to/{}'", options.valueMock.value_or("file"));
    }

    auto FilepathArgument::stringify(const fs::path &value) const -> std::string {
        return value.string();
    }

    auto FilepathArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<fs::path>> {
        std::vector<fs::path> result;
        const auto [begin, end] = parser.parameterRange(options.name);
        for (auto it = begin; it != end; ++it) {
            result.emplace_back(it->second);
        }
        return result;
    }

    auto FilepathArgument::registries() -> FilepathArgument {
        return FilepathArgument {
            {
                .name = "reg",
                .defaultSupplier = [] -> ResultMessage<fs::path> {
                    const auto env = registriesEnvVar();
                    if (!env) {
                        return ERR(fmt::format("No registries path was provided with the -reg parameter; "
                            "Environment variable {} was not set", REGISTRIES_ENV_VAR));
                    }
                    return *env;
                },
                .valueMock = "registries",
                .description = fmt::format("defaults to value of {} environment variable", REGISTRIES_ENV_VAR),
                .displayAsOptional = registriesEnvVar().has_value()
            }
        };
    }
}
