#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <ranges>
#include <result.hpp>
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_set.h>

namespace zvcr {

    [[nodiscard]]
    inline auto isNumber(const std::string &arg) -> bool {
        std::istringstream iss(arg);
        double number;
        iss >> number;
        return !iss.fail() && !iss.bad();
    }

    [[nodiscard]]
    inline auto isOption(const std::string &arg) -> bool {
        if (arg.empty() || isNumber(arg)) return false;
        return arg[0] == '-';
    }

    [[nodiscard]]
    inline auto badStream() -> std::istringstream {
        std::istringstream iss;
        iss.setstate(std::ios_base::failbit);
        return iss;
    }

    [[nodiscard]]
    inline auto trimLeadingDashes(const std::string &name) -> std::string {
        const auto pos = name.find_first_not_of('-');
        return std::string::npos != pos ? name.substr(pos) : name;
    }

    struct ArgumentParser {
        using ParameterMap = absl::btree_multimap<std::string, std::string>;
        using Parameter = std::pair<ParameterMap::const_iterator, ParameterMap::const_iterator>;

        std::vector<std::string> arguments;
        ParameterMap parameters;
        std::vector<std::string> remaining;
        absl::flat_hash_set<std::string> flags;
        
        ArgumentParser() = default;

        explicit ArgumentParser(const int argc, const char** argv) {
            parse(argc, argv);
        }

        auto parse(const int argc, const char** argv) -> void {
            arguments.clear();
            flags.clear();
            parameters.clear();
            remaining.clear();

            arguments.resize(static_cast<size_t>(argc - 1));
            for (size_t i = 1; i < argc; i++) {
                arguments[i - 1] = argv[i];
            }
            result::Option<std::string> lastParameter{};
            for (const auto &argument : arguments) {
                if (!isOption(argument)) {
                    if (lastParameter) {
                        parameters.emplace(*lastParameter, argument);
                    } else {
                        remaining.push_back(argument);
                    }
                    continue;
                }
                const auto name = trimLeadingDashes(argument);

                if (const auto equalPos = name.find('='); equalPos != std::string::npos) {
                    lastParameter = name.substr(0, equalPos);
                    parameters.emplace(*lastParameter, name.substr(equalPos + 1));
                    continue;
                }
                lastParameter = result::None;
                flags.emplace(name);
            }
        }

        [[nodiscard]]
        auto flag(const std::string_view name) const -> bool {
            return flags.contains(name);
        }

        [[nodiscard]]
        auto parameterRange(const std::string_view name) const -> Parameter {
            return parameters.equal_range(name);
        }

        [[nodiscard]]
        auto parameter(const std::string_view name) const -> std::vector<std::string> {
            const auto [begin, end] = parameterRange(name);
            std::vector<std::string> result;
            for (auto it = begin; it != end; ++it) {
                result.push_back(it->second);
            }
            return result;
        }

        [[nodiscard]]
        auto parameterKeys() const -> std::vector<std::string> {
            std::vector<std::string> result;
            result.reserve(parameters.size());

            std::unordered_set<std::string> seen;
            for (const auto &key : parameters | std::views::keys) {
                if (seen.insert(key).second)
                    result.push_back(key);
            }
            return result;
        }
    };

}
