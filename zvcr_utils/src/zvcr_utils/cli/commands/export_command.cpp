#include <zvcr_utils/cli/commands/export_command.hpp>
#include <zvcr_utils/cli/command_line.hpp>
#include <zvcr_utils/core/export.hpp>
#include <zvcr_utils/util/progress.hpp>
#include <zvcr_utils/util/thread_pool.hpp>
#include <zvcr_utils/util/io_utils.hpp>
#include <zvcr/io/file_location.hpp>
#include <zvcr/io/serialize/deserialize.hpp>
#include <zvcr_utils/util/registry_wrapper.hpp>

namespace zvcr {

    auto ExportCommand::execute(const CommandLine &cli) -> ErrorMessage {
        const auto &args = cli.parser;

        const auto &registriesPath = TRY(registriesArg.get(args));
        TRY(loadRegistryWrappers(registriesPath));

        const auto dimensionType = TRY(dimensionArg.get(args));
        const auto &sourcePaths = TRY(inArg.parseAll(args));
        auto sourcesResult = inputsArg.parseAsSource(args, sourcePaths, extension);
        if (!sourcesResult) return ERR(sourcesResult.error());

        const auto &sources = **sourcesResult;
        const auto &outPath = TRY(outArg.get(args));
        const auto threads = TRY(threadsArg.get(args));
        const auto epoch = TRY(epochArg.get(args));

        cli.log<mc::INFO>("Exporting regions in dimension {} from {} sources at epoch {}", dimensionName(dimensionType), sourcePaths.size(), epoch);
        cli.log<mc::INFO>("Sources (zvcr):");
        for (const auto &path : sourcePaths) {
            cli.log<mc::INFO>("- {}", path.string());
        }
        cli.log<mc::INFO>("Destination (mca): {}", outPath.string());
        cli.log<mc::INFO>("Using {} threads to export {} region files", threads, sources.size());

        const auto t0 = std::chrono::steady_clock::now();
        {
            Progress progress{.total = sources.size()};
            auto progressWatch = watchProgress(cli.logger, progress);
            ThreadPool threadPool{threads};
            for (const auto &pos : sources) {
                threadPool.enqueue([&, pos] {
                    const auto location = RegionLocation{pos.rx, pos.rz, dimensionType};
                    bool found{};
                    for (const auto &parentDirectoryIn : sourcePaths) {
                        const auto zvcrResult = readFileAt(parentDirectoryIn, location);
                        if (!zvcrResult) {
                            if (zvcrResult.error().type == FILE_NOT_FOUND) continue;

                            cli.log<mc::ERROR>("\nCould not read zvcr file at {}: {}; Skipping",
                                location.filePath(parentDirectoryIn).string(),
                                zvcrResult.error().what());
                            continue;
                        }
                        if (found) {
                            cli.log<mc::ERROR>("\nSkipping duplicate region file found within the provided sources: {}", location.filePath(parentDirectoryIn).string());
                            continue;
                        }
                        found = true;
                        try {
                            const auto region = createAnvilRegion(outPath, dimensionType, mc::anvil::Pos{.x = pos.rx, .z = pos.rz}, zvcrResult->region, epoch);
                            fs::create_directories(region.filePath().parent_path());
                            region.write();
                        } catch (const std::exception &e) {
                            cli.log<mc::ERROR>("\nCould not write mca file {}: {}; Skipping", location.fileName("mca"), e.what());
                        }
                    }
                    progress.increment();
                });
            }
            progressWatch.join();
        }
        const auto t1 = std::chrono::steady_clock::now();
        cli.log<mc::INFO>("Finished exporting zvcr files to mca in {} seconds", std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count());
        return {};
    }


}