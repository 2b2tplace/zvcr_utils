#pragma once

#include <memory>
#include <mc_cpp/logger.hpp>
#include <zvcr_utils/cli/parse_args.hpp>
#include <zvcr_utils/cli/commands/command_base.hpp>

namespace zvcr {

    inline constexpr std::string_view COMMAND = "zvcr";
    inline constexpr std::string_view VERSION = "0.0.0.0";

    struct CommandLine {
        ArgumentParser parser;
        mutable mc::Logger logger;
        std::vector<std::unique_ptr<Command>> commands;

        explicit CommandLine(int argc, const char **argv, std::ostream &out, bool enableLogPrefix);

        explicit CommandLine(int argc, const char **argv, bool enableLogPrefix);

        explicit CommandLine(int argc, const char **argv);

        [[nodiscard]]
        auto execute() const -> bool;

        template<mc::LogLevel L, typename... T>
        auto log(fmt::format_string<T...> fmt, T &&... args) const -> void {
            logger.log<L>(fmt, std::forward<T>(args)...);
        }
    };

}
