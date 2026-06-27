#include <zvcr_utils/cli/commands/merge_command.hpp>
#include <zvcr_utils/cli/command_line.hpp>
#include <zvcr_utils/core/merge.hpp>
#include <zvcr_utils/util/progress.hpp>
#include <zvcr_utils/util/thread_pool.hpp>
#include <zvcr_utils/util/io_utils.hpp>
#include <zvcr/io/file_location.hpp>
#include <zvcr/io/serialize/deserialize.hpp>
#include <mc_cpp/logger.hpp>

namespace zvcr {

    auto MergeCommand::execute(const CommandLine &cli) -> ErrorMessage {
        const auto &args = cli.parser;

        const auto dimensionType = TRY(dimensionArg.get(args));
        const auto &sourcePaths = TRY(inArg.parseAll(args));
        auto sourcesResult = inputsArg.parseAsSource(args, sourcePaths, extension);
        if (!sourcesResult) return ERR(sourcesResult.error());

        const auto &sources = *sourcesResult->get();
        const auto &outPath = TRY(outArg.get(args));
        const auto threads = TRY(threadsArg.get(args));
        const auto backupExisting = TRY(backupExistingArg.get(args));
        const auto zstdLevel = TRY(zstdLevelArg.get(args));
        const auto zstdThreads = TRY(zstdThreadsArg.get(args));

        cli.log<mc::INFO>("Merging regions in dimension {} from {} sources", dimensionName(dimensionType), sourcePaths.size());
        cli.log<mc::INFO>("Sources:");
        for (const auto &path : sourcePaths) {
            cli.log<mc::INFO>("- {}", path.string());
        }
        cli.log<mc::INFO>("Destination: {}", outPath.string());
        cli.log<mc::INFO>("Using {} threads to merge {} region files", threads, sources.size());

        const auto t0 = std::chrono::steady_clock::now();
        {
            Progress progress{.total = sources.size()};
            auto progressWatch = watchProgress(cli.logger, progress);
            ThreadPool threadPool{threads};
            for (const auto &pos : sources) {
                threadPool.enqueue([&, pos] {
                    const auto location = RegionLocation{pos.rx, pos.rz, dimensionType};
                    uint16_t firstProtocolVersion{};
                    std::vector<Region> regionsToMerge;
                    fs::path lastPath;
                    for (const auto &parentDirectoryIn : sourcePaths) {
                        const auto zvcrResult = readFileAt(parentDirectoryIn, location);
                        const auto filepath = location.filePath(parentDirectoryIn).string();
                        const auto protocolVersion = zvcrResult->protocolVersion;

                        if (!zvcrResult) {
                            if (zvcrResult.error().type == FILE_NOT_FOUND) continue;

                            cli.log<mc::ERROR>("\nCould not read {} file at {}: {}; Skipping",
                                extension, filepath, zvcrResult.error().what());
                            continue;
                        }
                        if (firstProtocolVersion != 0 && protocolVersion != firstProtocolVersion) {
                            cli.log<mc::ERROR>("\nUnable to merge regions varying in protocol version ({} != {}); Skipping {}",
                                protocolVersion, firstProtocolVersion, filepath);
                            continue;
                        }
                        if (firstProtocolVersion == 0)
                            firstProtocolVersion = protocolVersion;

                        regionsToMerge.push_back(zvcrResult->region);
                        lastPath = parentDirectoryIn;
                    }
                    if (regionsToMerge.empty()) {
                        progress.increment();
                        return;
                    }
                    // nothing to merge, just copy the region to the destination (or do nothing if already at destination)
                    if (regionsToMerge.size() == 1) {
                        if (lastPath != outPath) {
                            const auto src = location.filePath(lastPath);
                            const auto dst = location.filePath(outPath);
                            try {
                                if (backupExisting) backupIfExists(dst);
                                fs::create_directories(dst.parent_path());
                                fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
                            } catch (const fs::filesystem_error &e) {
                                cli.log<mc::ERROR>("\nCould not copy file: {}", e.what());
                            }
                        }
                        progress.increment();
                        return;
                    }
                    // merge potentially partial regions into one combined region
                    File result{.protocolVersion = firstProtocolVersion, .dimensionType = dimensionType};
                    bool mergedSegments{};
                    for (uint8_t x = 0; x < REGION_SIDELENGTH_SEGMENTS; x++) {
                        for (uint8_t z = 0; z < REGION_SIDELENGTH_SEGMENTS; z++) {
                            mergedSegments |= mergeSegments(regionsToMerge, result.region, x, z, dimensionType);
                        }
                    }
                    if (mergedSegments)
                        writeFileSafelyLog(cli.logger, result, outPath, location, backupExisting, zstdLevel, zstdThreads);

                    progress.increment();
                });
            }
            progressWatch.join();
        }
        const auto t1 = std::chrono::steady_clock::now();
        cli.log<mc::INFO>("Finished merging {} files in {} seconds", extension,
            std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count());

        return {};
    }

}
