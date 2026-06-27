#include <zvcr_utils/cli/commands/help_command.hpp>

namespace zvcr {
    auto HelpCommand::execute(const CommandLine &cli) -> ErrorMessage {
        if (cli.parser.remaining.empty()) {
            for (const auto &command : cli.commands) {
                cli.log<mc::INFO>("{}", command->explain().at(0));
            }
            return {};
        }
        cli.log<mc::INFO>("Basic parameter syntax:");
        cli.log<mc::INFO>("    -parameter=value");
        cli.log<mc::INFO>("    -parameter='string value with spaces'");
        cli.log<mc::INFO>("    -flag");
        cli.log<mc::INFO>("Optional parameters are denoted with []:");
        cli.log<mc::INFO>("    [-optional=value]");
        cli.log<mc::INFO>("Variadic parameters are denoted with [...]:");
        cli.log<mc::INFO>("    -parameters=values[...] (example: -parameters=value1 value2 value3)");

        const auto &commandName = cli.parser.remaining.at(0);
        for (const auto &command : cli.commands) {
            if (command->name() != commandName) continue;

            for (const auto &line : command->explain()) {
                cli.log<mc::INFO>("{}", line);
            }
            return {};
        }
        return ERR(fmt::format("Unknown command '{}'. Use `{} -help` to see all available commands.", commandName, COMMAND));
    }
}
