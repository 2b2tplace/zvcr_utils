#include <zvcr_utils/cli/arguments/string_argument.hpp>

namespace zvcr {
    auto StringArgument::mock() const -> std::string {
        return fmt::format("='{}'", options.valueMock.value_or("value"));
    }

    auto StringArgument::stringify(const std::string &value) const -> std::string {
        return value;
    }

    auto StringArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<std::string>> {
        return parser.parameter(options.name);
    }
}
