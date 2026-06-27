#pragma once

#include <zvcr_utils/cli/commands/command_base.hpp>
#include <zvcr_utils/cli/arguments/filepath_argument.hpp>
#include <zvcr_utils/cli/arguments/dimension_argument.hpp>
#include <zvcr_utils/cli/arguments/string_argument.hpp>
#include <zvcr_utils/cli/arguments/vec3d_argument.hpp>

namespace zvcr {
    class LevelDatCommand final : public Command {
    public:
        LevelDatCommand(): Command("level-dat", "Creates a level.dat file with custom parameters in a Minecraft world folder") {}

        auto execute(const CommandLine &cli) -> ErrorMessage override;
    private:
        FilepathArgument worldFolderArg = argument(FilepathArgument{{
            .name = "path",
            .valueMock = "minecraft-world-folder"
        }});
        StringArgument nameArg = argument(StringArgument{{
            .name = "name",
            .valueMock = "name of the world",
            .description = "defaults to name of the world folder",
            .displayAsOptional = true
        }});
        DimensionArgument spawnDimensionArg = argument(DimensionArgument{{
            .name = "spawn-dim",
            .defaultValue = DimensionType::OVERWORLD
        }});
        Vec3dArgument spawnPositionArg = argument(Vec3dArgument{{
            .name = "spawn-pos",
            .defaultValue = mc::Vec3d{}
        }});
        FilepathArgument registriesArg = argument(FilepathArgument::registries());
    };
}
