#include <zvcr_utils/util/io_utils.hpp>

namespace zvcr {
    auto backupIfExists(const fs::path &filepath) -> void {
        if (fs::exists(filepath))
            fs::rename(filepath, filepath.string() + ".backup_" + std::to_string(time(nullptr)));
    }

    auto writeFileSafely(const fs::path &path, const bool backupExistingFiles,
                         const std::function<auto(const fs::path &) -> ErrorMessage> &writeFile) -> ErrorMessage {
        try {
            auto temporaryPath = path;
            temporaryPath.replace_extension(temporaryPath.extension().string() + ".tmp");

            fs::create_directories(path.parent_path());

            if (backupExistingFiles) backupIfExists(path);

            TRY(writeFile(temporaryPath));
#if defined(_WIN32)
            fs::remove(finalPath);
#endif
            fs::rename(temporaryPath, path);
            return {};
        } catch (const fs::filesystem_error &e) {
            return ERR(e.what());
        }
    }

    auto writeFileSafely(const File &zvcrFile, const fs::path &parentDirectoryOut, const RegionLocation &location,
                         const bool backupExistingFiles, const int compressionLevel, const uint compressionThreads) -> ErrorMessage {
        return writeFileSafely(location.filePath(parentDirectoryOut), backupExistingFiles, [&](const fs::path &path) -> ErrorMessage {
            TRY(writeFile(zvcrFile, path, compressionLevel, compressionThreads));
            return {};
        });
    }

    auto writeFileSafelyLog(mc::Logger &log, const File &zvcrFile, const fs::path &parentDirectoryOut,
                            const RegionLocation &location, const bool backupExistingFiles,
                            const int compressionLevel, const uint compressionThreads) -> bool {
        if (const auto result = writeFileSafely(zvcrFile, parentDirectoryOut, location, backupExistingFiles, compressionLevel, compressionThreads); !result) {
            log.log<mc::ERROR>("Could not save zvcr file at {}: {}", location.filePath(parentDirectoryOut).string(), result.error());
            return false;
        }
        return true;
    }
}
