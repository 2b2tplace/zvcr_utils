#include <iostream>
#include <zvcr_utils/cli/command_line.hpp>
#include <zvcr_utils/cli/commands/help_command.hpp>
#include <zvcr_utils/cli/commands/merge_command.hpp>
#include <zvcr_utils/cli/commands/export_command.hpp>

namespace zvcr {

    CommandLine::CommandLine(const int argc, const char **argv, std::ostream &out, const bool enableLogPrefix): logger(out, enableLogPrefix) {
        parser.parse(argc, argv);
        commands.push_back(std::make_unique<HelpCommand>());
        commands.push_back(std::make_unique<MergeCommand>());
        commands.push_back(std::make_unique<ExportCommand>());
    }

    CommandLine::CommandLine(const int argc, const char **argv, const bool enableLogPrefix): CommandLine(argc, argv, std::cout, enableLogPrefix) {}

    CommandLine::CommandLine(const int argc, const char **argv): CommandLine(argc, argv, std::cout, false) {
        logger.enableLogPrefix = parser.flag("enable-log-prefix");
    }

    auto CommandLine::execute() const -> bool {
        log<mc::INFO>("{} command line utility (version {})", COMMAND, VERSION);
        for (const auto &[arg, val] : parser.parameters) {
            log<mc::DEBUG>("Parameter: {} = {}", arg, val);
        }
        for (const auto &arg : parser.flags) {
            log<mc::DEBUG>("Flag: {}", arg);
        }
        for (const auto &arg : parser.remaining) {
            log<mc::DEBUG>("Remaining: {}", arg);
        }
        for (const auto &command : commands) {
            if (!parser.flag(command->name())) continue;

            try {
                if (const auto result = command->execute(*this); !result) {
                    log<mc::ERROR>("{}", result.error());
                    log<mc::ERROR>("Usage: ");
                    for (const auto &line : command->explain()) {
                        log<mc::ERROR>("{}", line);
                    }
                    return false;
                }
            } catch (const std::exception &e) {
                log<mc::FATAL>("Unexpected error: {}", e.what());
                return false;
            }
            return true;
        }
        if (!parser.arguments.empty()) {
            log<mc::WARN>("Invalid command usage, arguments were ignored.");
            return true;
        }
        log<mc::INFO>("Use `{} -help` to see all available commands.", COMMAND);
        return true;
    }

}
