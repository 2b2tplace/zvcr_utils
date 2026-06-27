#include <zvcr_utils/cli/arguments/range_argument.hpp>

namespace zvcr {

    auto RangeArgument::mock() const -> std::string {
        return fmt::format("={}", options.valueMock.value_or("n (single) or a-b (range)"));
    }

    auto RangeArgument::stringify(const Range &value) const -> std::string {
        if (value.first == value.second) return std::to_string(value.first);

        return fmt::format("{}-{}", value.first, value.second);
    }

    auto RangeArgument::parseRange(const std::string &rangeStr) -> ResultMessage<Range> {
        const auto dash = rangeStr.find('-');
        if (dash == std::string::npos) {
            try {
                const auto single = std::stoi(rangeStr);
                return Range{single, single};
            } catch (const std::runtime_error &e) {
                return ERR(fmt::format("Invalid single range '{}': {}", rangeStr, e.what()));
            }
        }
        try {
            const auto start = std::max(0, std::stoi(rangeStr.substr(0, dash)));
            const auto end = std::max(0, std::stoi(rangeStr.substr(dash + 1)));

            return Range{start, end};
        } catch (const std::runtime_error &e) {
            return ERR(fmt::format("Invalid range '{}': {}", rangeStr, e.what()));
        }
    }

    auto RangeArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<Range>> {
        std::vector<Range> result;
        const auto [begin, end] = parser.parameterRange(options.name);

        for (auto it = begin; it != end; ++it) {
            result.push_back(TRY(parseRange(it->second)));
        }
        return result;
    }


}