#pragma once

#include <string>
#include <result.hpp>
#include <variant>
#include <filesystem>

namespace zvcr {

    template<typename T>
    using ResultMessage = result::Result<T, std::string>;

    using ErrorMessage = ResultMessage<std::monostate>;

    namespace fs = std::filesystem;

    using Path = fs::path;
    using Paths = std::vector<Path>;
    using Sources = std::variant<Path, Paths>;

}
