#pragma once

#include <string>
#include <result.hpp>
#include <fmt/format.h>
#include <zvcr_utils/cli/parse_args.hpp>
#include <zvcr_utils/types.hpp>

namespace zvcr {

    template<typename T>
    struct ArgumentOptions {
        std::string name;
        result::Option<T> defaultValue;
        result::Option<std::function<auto() -> ResultMessage<T>>> defaultSupplier;
        result::Option<std::string> valueMock;
        result::Option<std::string> description;
        bool variadic{};
        bool displayAsOptional{};
    };

    template<typename T>
    struct Argument {
        using Options = ArgumentOptions<T>;
        Options options;

        Argument(const Argument &other): options(other.options) {}

        explicit Argument(Options options): options(std::move(options)) {}

        virtual ~Argument() = default;

        [[nodiscard]]
        virtual auto mock() const -> std::string = 0;

        [[nodiscard]]
        virtual auto usage() const -> std::string {
            return fmt::format("-{}{}{}", options.name, mock(), options.variadic ? "[...]" : "");
        }

        [[nodiscard]]
        virtual auto explain() const -> std::string {
            auto explanation = usage();
            if (options.description)
                explanation += fmt::format(" ({})", *options.description);

            if (options.displayAsOptional)
                return fmt::format("[{}]", explanation);

            if (!options.defaultValue)
                return explanation;

            if constexpr (Eq<T>::value) {
                if (*options.defaultValue == T{})
                    return fmt::format("[{}]", explanation);
            }
            return fmt::format("[{}] (defaults to {})", explanation, stringify(*options.defaultValue));
        }

        [[nodiscard]]
        virtual auto stringify(const T &value) const -> std::string = 0;

        [[nodiscard]]
        virtual auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<T>> = 0;

        [[nodiscard]]
        virtual auto parse(const ArgumentParser &parser) const -> ResultMessage<result::Option<T>> {
            const auto vector = TRY(parseAll(parser));
            if (vector.empty()) return result::None;

            return vector[0];
        }

        [[nodiscard]]
        virtual auto get(const ArgumentParser &parser) const -> ResultMessage<T> {
            const auto result = TRY(parse(parser));
            if (result) return *result;

            if (options.defaultValue)
                return *options.defaultValue;

            if (options.defaultSupplier)
                return (*options.defaultSupplier)();

            return ERR(fmt::format("Missing argument '{}'. Usage: {}", options.name, usage()));
        }

    private:
        template<typename, typename = std::void_t<>>
        struct Eq : std::false_type {};

        template<typename V>
        struct Eq<V, std::void_t<decltype(std::declval<V>() == std::declval<V>())>> : std::true_type {};
    };

}
