#include <zvcr_utils/cli/arguments/vec3d_argument.hpp>

namespace zvcr {
    auto Vec3dArgument::mock() const -> std::string {
        return fmt::format("={}", options.valueMock.value_or("x,y,z"));
    }

    auto Vec3dArgument::stringify(const mc::Vec3d &value) const -> std::string {
        return fmt::format("{}, {}, {}", value.x, value.y, value.z);
    }

    auto Vec3dArgument::parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<mc::Vec3d>> {
        std::vector<mc::Vec3d> result;
        const auto [begin, end] = parser.parameterRange(options.name);

        for (auto it = begin; it != end; ++it) {
            const auto &positionStr = it->second;
            std::vector<double> positionVector;
            std::stringstream ss(positionStr);
            std::string token;

            while (std::getline(ss, token, ',')) {
                try {
                    positionVector.push_back(std::stod(token));
                } catch (const std::runtime_error &e) {
                    return ERR(fmt::format("Invalid coordinates in position: {}", e.what()));
                }
            }
            if (positionVector.size() != 3)
                return ERR("Invalid position provided, expected 3 real values.");

            result.emplace_back(positionVector[0], positionVector[1], positionVector[2]);
        }
        return result;
    }
}
