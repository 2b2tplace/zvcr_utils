#pragma once

#include <zvcr_utils/cli/commands/command_base.hpp>
#include <zvcr_utils/cli/arguments/filepath_argument.hpp>
#include <zvcr_utils/cli/arguments/generic_argument.hpp>
#include <zvcr_utils/cli/arguments/dimension_argument.hpp>
#include <zvcr_utils/cli/arguments/region_source_argument.hpp>
#include <zvcr_utils/cli/arguments/flag_argument.hpp>

namespace zvcr {
    class MergeCommand final : public Command {
    public:
        MergeCommand(): Command("merge", "Merges distinct zvcr directories together, preserving their histories and combining their timelines") {}

        auto execute(const CommandLine &cli) -> ErrorMessage override;
    private:
        DimensionArgument dimensionArg = argument(DimensionArgument::create());
        FilepathArgument inArg = argument(FilepathArgument{{
            .name = "in",
            .valueMock = "input-zvcr-dirs",
            .variadic = true
        }});
        FilepathArgument outArg = argument(FilepathArgument{{
            .name = "out",
            .valueMock = "output-zvcr-dir"
        }});
        RegionSourceArgument inputsArg = argument(RegionSourceArgument::create());
        GenericArgument<int> zstdLevelArg = argument(zstdCompressionLevelArgument());
        GenericArgument<uint> zstdThreadsArg = argument(zstdCompressionThreadsArgument());
        GenericArgument<size_t> threadsArg = argument(threadsArgument());
        FlagArgument backupExistingArg = argument(backupExistingArgument());
    };
}
