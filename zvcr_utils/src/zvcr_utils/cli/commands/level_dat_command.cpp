#include <zvcr_utils/cli/commands/level_dat_command.hpp>
#include <zvcr_utils/util/registry_wrapper.hpp>
#include <zvcr_utils/cli/command_line.hpp>
#include <mc_cpp/anvil/level.hpp>

namespace zvcr {

    auto LevelDatCommand::execute(const CommandLine &cli) -> ErrorMessage {
        const auto &args = cli.parser;

        const auto &registriesPath = TRY(registriesArg.get(args));
        TRY(loadRegistryWrappers(registriesPath));

        const auto &outPath = TRY(worldFolderArg.get(args));
        const auto &levelName = TRY(nameArg.parse(args)).value_or(outPath.filename().string());
        const auto dimensionType = TRY(spawnDimensionArg.get(args));
        const auto position = TRY(spawnPositionArg.get(args));

        cli.log<mc::INFO>("Saving level.dat file in path {} with world name {}", outPath.string(), levelName);
        cli.log<mc::INFO>("Using spawn position {}, {}, {} in {}", position.x, position.y, position.z, dimensionName(dimensionType));

        const auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        const auto &registry = getRegistry(mc::RELEASE_1_21_4);
        auto level = mc::Level::createStaticEmptyLevel(levelName, registry);
        level.player.position = position;
        level.player.dimension = static_cast<mc::DimensionType>(dimensionType);
        level.lastPlayed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();

        fs::create_directories(outPath);
        level.write(outPath / "level.dat");
        return {};
    }


}
