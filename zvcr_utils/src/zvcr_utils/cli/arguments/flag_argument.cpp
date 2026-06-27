#include <zvcr_utils/cli/arguments/flag_argument.hpp>

namespace zvcr {
    auto FlagArgument::mock() const -> std::string {
        return "";
    }

    auto FlagArgument::stringify(const bool &value) const -> std::string {
        return value ? "true" : "false";
    }

    auto FlagArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<bool>> {
        if (parser.flag(options.name)) return std::vector{true};

        std::vector<bool> result;
        const auto [begin, end] = parser.parameterRange(options.name);

        for (auto it = begin; it != end; ++it) {
            const auto &val = it->second;
            std::string lower;
            std::ranges::transform(val, lower.begin(), [](const auto c) {
                return std::tolower(c);
            });
            result.push_back(lower == "true" || lower == "yes" || lower == "1");
        }
        return result;
    }

    auto FlagArgument::create(const std::string &name) -> FlagArgument {
        return FlagArgument{{.name = name, .defaultValue = false}};
    }

    auto backupExistingArgument() -> FlagArgument {
        return FlagArgument::create("backup-existing");
    }

}
