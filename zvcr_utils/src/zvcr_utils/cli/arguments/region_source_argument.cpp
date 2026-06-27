#include <zvcr_utils/cli/arguments/region_source_argument.hpp>

namespace zvcr {
    auto RegionSourceArgument::mock() const -> std::string {
        return fmt::format("={}", options.valueMock.value_or("x.z or min_x.min_z,max_x.max_z"));
    }

    auto RegionSourceArgument::stringify(const Rect &value) const -> std::string {
        return fmt::format("[min_x = {}, min_z = {}, max_x = {}, max_z = {}]",
                           value.minX, value.minZ, value.maxX, value.maxZ);
    }

    auto RegionSourceArgument::parseCoordinates(const std::string &pairStr) -> ResultMessage<RegionCoordinates> {
        const auto dot = pairStr.find('.');
        if (dot == std::string::npos)
            return ERR(fmt::format("Invalid region coordinate format: Missing '.' in {}", pairStr));

        try {
            const auto x = std::stoi(pairStr.substr(0, dot));
            const auto z = std::stoi(pairStr.substr(dot + 1));
            return RegionCoordinates{x, z};
        } catch (const std::runtime_error &e) {
            return ERR(fmt::format("Invalid region coordinates: {}", e.what()));
        }
    }

    auto RegionSourceArgument::parseRect(const std::string &rectStr) -> ResultMessage<Rect> {
        const auto comma = rectStr.find(',');
        if (comma == std::string::npos) return Rect::singleton(TRY(parseCoordinates(rectStr)));

        const auto [minX, minZ] = TRY(parseCoordinates(rectStr.substr(0, comma)));
        const auto [maxX, maxZ] = TRY(parseCoordinates(rectStr.substr(comma + 1)));
        return Rect{minX, minZ, maxX, maxZ};
    }

    auto RegionSourceArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<Rect>> {
        std::vector<Rect> result;
        const auto [begin, end] = parser.parameterRange(options.name);

        for (auto it = begin; it != end; ++it) {
            result.push_back(TRY(parseRect(it->second)));
        }
        return result;
    }

    auto RegionSourceArgument::parseAsSource(const ArgumentParser &parser, const std::vector<fs::path> &rootDirs,
                                             const std::string_view extension) const -> ResultMessage<std::unique_ptr<RegionSource>> {
        const auto parsed = TRY(parseAll(parser));
        if (!parsed.empty())
            return std::make_unique<RegionRectVector>(parsed);

        if (rootDirs.empty())
            return ERR("No region source paths were provided.");

        return std::make_unique<RegionMultiDirectory>(rootDirs, extension);
    }

    auto RegionSourceArgument::create() -> RegionSourceArgument {
        return RegionSourceArgument {{
            .name = "inputs",
            .description = "defaults to all region files in the input directories",
            .variadic = true,
            .displayAsOptional = true
        }};
    }
}
