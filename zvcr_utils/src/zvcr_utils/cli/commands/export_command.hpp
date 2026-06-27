#pragma once

#include <zvcr_utils/cli/commands/command_base.hpp>
#include <zvcr_utils/cli/arguments/filepath_argument.hpp>
#include <zvcr_utils/cli/arguments/generic_argument.hpp>
#include <zvcr_utils/cli/arguments/dimension_argument.hpp>
#include <zvcr_utils/cli/arguments/region_source_argument.hpp>

namespace zvcr {
    class ExportCommand final : public Command {
    public:
        ExportCommand(): Command("export", "Exports zvcr files to Minecraft anvil (mca)") {}

        auto execute(const CommandLine &cli) -> ErrorMessage override;
    private:
        DimensionArgument dimensionArg = argument(DimensionArgument::create());
        FilepathArgument inArg = argument(FilepathArgument{{
            .name = "in",
            .valueMock = "zvcr-files",
            .variadic = true
        }});
        FilepathArgument outArg = argument(FilepathArgument{{
            .name = "out",
            .valueMock = "minecraft-world-folder"
        }});
        RegionSourceArgument inputsArg = argument(RegionSourceArgument::create());
        TimestampArgument epochArg = argument(epochArgument());
        GenericArgument<size_t> threadsArg = argument(threadsArgument());
        FilepathArgument registriesArg = argument(FilepathArgument::registries());
    };
}
