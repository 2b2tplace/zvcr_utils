#include <zvcr_utils/cli/arguments/dimension_argument.hpp>

namespace zvcr {
    auto DimensionArgument::mock() const -> std::string {
        return fmt::format("={}", options.valueMock.value_or("{overworld/nether/end}"));
    }

    auto DimensionArgument::stringify(const DimensionType &value) const -> std::string {
        switch (value) {
            case DimensionType::OVERWORLD: return "overworld";
            case DimensionType::NETHER: return "nether";
            case DimensionType::THE_END: return "end";
            default: std::unreachable();
        }
    }

    auto DimensionArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<DimensionType>> {
        std::vector<DimensionType> result;
        const auto [begin, end] = parser.parameterRange(options.name);
        for (auto it = begin; it != end; ++it) {
            const auto &val = it->second;

            if (val == "overworld") {
                result.push_back(DimensionType::OVERWORLD);
            } else if (val == "nether" || val == "the_nether") {
                result.push_back(DimensionType::NETHER);
            } else if (val == "end" || val == "the_end") {
                result.push_back(DimensionType::THE_END);
            } else {
                return ERR(fmt::format("Invalid dimension '{}'. Valid options are: "
                    "'overworld', 'nether' or 'the_nether', 'end' or 'the_end'", val));
            }
        }
        return result;
    }

    auto DimensionArgument::create() -> DimensionArgument {
        return DimensionArgument {{.name = "dim"}};
    }
}
