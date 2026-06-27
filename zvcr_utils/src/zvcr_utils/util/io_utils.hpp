#pragma once

#include <zvcr_utils/types.hpp>
#include <zvcr/io/serialize/serialize.hpp>
#include <mc_cpp/logger.hpp>

namespace zvcr {

    namespace fs = std::filesystem;

    auto backupIfExists(const fs::path &filepath) -> void;

    auto writeFileSafely(const fs::path &path, bool backupExistingFiles,
                         const std::function<auto(const fs::path&) -> ErrorMessage> &writeFile) -> ErrorMessage;

    auto writeFileSafely(const File &zvcrFile, const fs::path &parentDirectoryOut,
                         const RegionLocation &location, bool backupExistingFiles,
                         int compressionLevel = ZSTD_COMPRESSION_LEVEL_DEFAULT,
                         uint compressionThreads = ZSTD_COMPRESSION_THREADS_DEFAULT) -> ErrorMessage;

    auto writeFileSafelyLog(mc::Logger &log, const File &zvcrFile, const fs::path &parentDirectoryOut,
                            const RegionLocation &location, bool backupExistingFiles,
                            int compressionLevel = ZSTD_COMPRESSION_LEVEL_DEFAULT,
                            uint compressionThreads = ZSTD_COMPRESSION_THREADS_DEFAULT) -> bool;

}
