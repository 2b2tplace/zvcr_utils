#include <zvcr_utils/cli/commands/import_command.hpp>
#include <zvcr_utils/cli/command_line.hpp>
#include <zvcr_utils/util/registry_wrapper.hpp>
#include <zvcr_utils/core/import.hpp>
#include <zvcr_utils/util/progress.hpp>
#include <zvcr_utils/util/thread_pool.hpp>
#include <zvcr_utils/util/io_utils.hpp>
#include <zvcr/io/file_location.hpp>
#include <zvcr/io/serialize/serialize.hpp>

namespace zvcr {

    auto ImportCommand::execute(const CommandLine &cli) -> ErrorMessage {
        const auto &args = cli.parser;

        const auto &registriesPath = TRY(registriesArg.get(args));
        TRY(loadRegistryWrappers(registriesPath));

        const auto dimensionType = TRY(dimensionArg.get(args));
        const auto &sourcePaths = TRY(inArg.parseAll(args));
        auto sourcesResult = inputsArg.parseAsSource(args, sourcePaths, "mca");
        if (!sourcesResult) return ERR(sourcesResult.error());

        const auto &sources = **sourcesResult;
        const auto &outPath = TRY(outArg.get(args));
        const auto threads = TRY(threadsArg.get(args));
        const auto overrideEpoch = TRY(overrideEpochArg.parse(args));
        const auto backupExisting = TRY(backupExistingArg.get(args));
        const auto zstdLevel = TRY(zstdLevelArg.get(args));
        const auto zstdThreads = TRY(zstdThreadsArg.get(args));

        cli.log<mc::INFO>("Importing dimension {}", dimensionName(dimensionType));
        cli.log<mc::INFO>("Sources (mca):");
        for (const auto &path : sourcePaths) {
            cli.log<mc::INFO>("- {}", path.string());
        }
        cli.log<mc::INFO>("Destination (zvcr): {}", outPath.string());
        cli.log<mc::INFO>("Using {} threads to export {} region files", threads, sources.size());

        const auto t0 = std::chrono::steady_clock::now();
        const auto &registry = getRegistryWrapper(PROTOCOL_VERSION);
        {
            Progress progress{.total = sources.size()};
            auto progressWatch = watchProgress(cli.logger, progress);
            ThreadPool threadPool{threads};
            for (const auto &pos : sources) {
                threadPool.enqueue([&, pos] {
                    bool found{};
                    for (const auto &parentDirectoryIn : sourcePaths) {
                        const auto location = RegionLocation{pos.rx, pos.rz, dimensionType};
                        mc::anvil::Region region{parentDirectoryIn, static_cast<mc::DimensionType>(dimensionType), mc::anvil::Pos{pos.rx, pos.rz}};
                        if (const auto readResult = region.read(); readResult != mc::anvil::RegionReadResult::OK) {
                            if (readResult == mc::anvil::RegionReadResult::FILE_HANDLE_CLOSED
                                || readResult == mc::anvil::RegionReadResult::EMPTY_REGION_FILE) continue;

                            cli.log<mc::ERROR>("\nCould not read mca region file at {}: Error code {}; Skipping",
                                region.filePath().string(), static_cast<int>(readResult));
                            continue;
                        }
                        if (found) {
                            cli.log<mc::ERROR>("\nSkipping duplicate region file found within the provided sources: {}", location.filePath(parentDirectoryIn).string());
                            continue;
                        }
                        found = true;
                        File file{.protocolVersion = static_cast<uint16_t>(registry.mc.protocolVersion()), .dimensionType = dimensionType};
                        if (const auto result = createZVCRRegion(registry.mc, dimensionType, region, file.region, overrideEpoch); !result) {
                            cli.log<mc::ERROR>("\nFailed to import mca region file: {}", result.error());
                            continue;
                        }
                        writeFileSafelyLog(cli.logger, file, outPath, location, backupExisting, zstdLevel, zstdThreads);
                    }
                    progress.increment();
                });
            }
            progressWatch.join();
        }
        const auto t1 = std::chrono::steady_clock::now();
        cli.log<mc::INFO>("Finished importing mca files to zvcr in {} seconds", std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count());
        return {};
    }

}
