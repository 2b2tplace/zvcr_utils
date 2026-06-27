#pragma once

#include <zvcr_utils/cli/commands/command_base.hpp>
#include <zvcr_utils/cli/arguments/filepath_argument.hpp>
#include <zvcr_utils/cli/arguments/generic_argument.hpp>
#include <zvcr_utils/cli/arguments/dimension_argument.hpp>
#include <zvcr_utils/cli/arguments/region_source_argument.hpp>
#include <zvcr_utils/cli/arguments/flag_argument.hpp>

namespace zvcr {
    class ImportCommand final : public Command {
    public:
        ImportCommand(): Command("import", "Imports Minecraft anvil (mca) files to zvcr") {}

        auto execute(const CommandLine &cli) -> ErrorMessage override;
    private:
        DimensionArgument dimensionArg = argument(DimensionArgument::create());
        FilepathArgument inArg = argument(FilepathArgument{{
            .name = "in",
            .valueMock = "minecraft-world-folder",
            .variadic = true
        }});
        FilepathArgument outArg = argument(FilepathArgument{{
            .name = "out",
            .valueMock = "zvcr-files"
        }});
        RegionSourceArgument inputsArg = argument(RegionSourceArgument::create());
        TimestampArgument overrideEpochArg = argument(TimestampArgument{{
            .name = "override-epoch",
            .valueMock = "a unix timestamp",
            .description = "in seconds, to override the timestamp of all snapshots; defaults to the recorded timestamp of each chunk",
            .displayAsOptional = true
        }});
        GenericArgument<int> zstdLevelArg = argument(zstdCompressionLevelArgument());
        GenericArgument<uint> zstdThreadsArg = argument(zstdCompressionThreadsArgument());
        GenericArgument<size_t> threadsArg = argument(threadsArgument());
        FlagArgument backupExistingArg = argument(backupExistingArgument());
        FilepathArgument registriesArg = argument(FilepathArgument::registries());
    };
}
