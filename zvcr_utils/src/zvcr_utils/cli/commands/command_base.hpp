#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <zvcr_utils/types.hpp>

namespace zvcr {

    struct CommandLine;

    class Command {
    protected:
        std::string name_;
        std::vector<std::string> explanation;
    public:
        explicit Command(const std::string_view name, const std::string_view description):
            name_(name), explanation({fmt::format("-{}: {}", name, description)}) {}

        virtual ~Command() = default;

        virtual auto execute(const CommandLine &cli) -> ErrorMessage = 0;

        [[nodiscard]]
        auto name() const -> std::string_view {
            return name_;
        }

        template<typename Arg>
        auto argument(const Arg &argument) -> Arg {
            explanation.push_back(fmt::format("    {}", argument.explain()));
            return argument;
        }

        [[nodiscard]]
        auto explain() const -> std::vector<std::string> {
            return explanation;
        }
    };

}
