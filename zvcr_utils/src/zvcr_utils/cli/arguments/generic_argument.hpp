#pragma once

#include <zvcr_utils/cli/arguments/argument_base.hpp>
#include <sstream>

namespace zvcr {

    template<typename, typename = void>
    struct Streamable : std::false_type {};

    template<typename T>
    struct Streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

    template<typename T>
    struct GenericArgument final : Argument<T> {
        static_assert(Streamable<T>::value, "Unsupported type");

        explicit GenericArgument(ArgumentOptions<T> options): Argument<T>(std::move(options)) {}

        explicit GenericArgument(const Argument<T> &other): Argument<T>(other) {}

        [[nodiscard]]
        auto mock() const -> std::string override {
            return fmt::format("={}", this->options.valueMock.value_or("value"));
        }

        [[nodiscard]]
        auto stringify(const T &value) const -> std::string override {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }

        [[nodiscard]]
        auto parseAll(const ArgumentParser &parser) const -> ResultMessage<std::vector<T>> override {
            std::vector<T> result;
            const auto [begin, end] = parser.parameterRange(this->options.name);

            for (auto it = begin; it != end; ++it) {
                const auto &val = it->second;
                T t{};
                std::istringstream iss{val};
                iss >> t;
                result.push_back(t);
            }
            return result;
        }
    };

    using TimestampArgument = GenericArgument<time_t>;

    auto epochArgument() -> TimestampArgument;

    auto threadsArgument() -> GenericArgument<size_t>;

    auto zstdCompressionLevelArgument() -> GenericArgument<int>;

    auto zstdCompressionThreadsArgument() -> GenericArgument<uint>;
}
