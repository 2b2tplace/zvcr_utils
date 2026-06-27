#pragma once

#include <zvcr_utils/cli/commands/command_base.hpp>
#include <zvcr_utils/cli/command_line.hpp>

namespace zvcr {

    class HelpCommand final : public Command {
    public:
        HelpCommand(): Command("help", fmt::format("Shows all commands and their usages. Use `{} -help <command>` to see the usage of that command.", COMMAND)) {}

        auto execute(const CommandLine &cli) -> ErrorMessage override;
    };

}
