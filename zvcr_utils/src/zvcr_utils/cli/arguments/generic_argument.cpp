#include <thread>
#include <zvcr_utils/cli/arguments/generic_argument.hpp>
#include <zvcr/io/compression.hpp>

namespace zvcr {
    auto epochArgument() -> TimestampArgument {
        return TimestampArgument {
            {
                .name = "epoch",
                .defaultValue = time(nullptr),
                .valueMock = "a unix timestamp",
                .description = "in seconds; defaults to current time"
            }
        };
    }

    auto threadsArgument() -> GenericArgument<size_t> {
        return GenericArgument<size_t> {
            {
                .name = "threads",
                .defaultValue = static_cast<size_t>(std::thread::hardware_concurrency()),
                .valueMock = "number of threads",
                .description = "defaults to number of threads on this system"
            }
        };
    }

    auto zstdCompressionLevelArgument() -> GenericArgument<int> {
        return GenericArgument<int> {
                {
                    .name = "zstd-level",
                    .defaultValue = ZSTD_COMPRESSION_LEVEL_DEFAULT,
                    .valueMock = "zstd compression level"
                }
        };
    }

    auto zstdCompressionThreadsArgument() -> GenericArgument<uint> {
        return GenericArgument<uint> {
            {
                .name = "zstd-threads",
                .defaultValue = ZSTD_COMPRESSION_THREADS_DEFAULT,
                .valueMock = "zstd compression threads"
            }
        };
    }

}
