#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <mc_cpp/logger.hpp>

namespace zvcr {

    class Progress {
    public:
        std::atomic<size_t> current{};
        size_t total{};
        std::chrono::steady_clock::time_point startedTimestamp{std::chrono::steady_clock::now()};

        auto increment() -> void;

        [[nodiscard]]
        auto percentage() const -> float;

        [[nodiscard]]
        auto completed() const -> bool;

        [[nodiscard]]
        auto elapsed() const -> std::chrono::steady_clock::duration;
    };

    [[nodiscard]]
    auto formatDuration(std::chrono::seconds secs) -> std::string ;

    [[nodiscard]]
    auto formatRemainingETA(const Progress &progress) -> std::string ;

    template<mc::LogLevel L, size_t width>
    auto defaultProgressPrinter(mc::Logger &logger, const Progress &progress) -> void {
        std::string bar(width, ' ');
        const auto fill = static_cast<size_t>(progress.percentage() * width);
        std::fill_n(bar.begin(), fill, '=');
        if (fill > 0 && fill < width) bar[fill] = '>';

        logger.print(fmt::format("[{}] {:.2f}% ({} / {}); Elapsed: {}, Remaining: {}",
            bar, progress.percentage() * 100,
            progress.current.load(), progress.total,
            formatDuration(std::chrono::duration_cast<std::chrono::seconds>(progress.elapsed())),
            formatRemainingETA(progress)
        ), mc::logLevelToColor(mc::INFO));
    }

    using ProgressPrinter = std::function<auto(mc::Logger&, const Progress&) -> void>;

    static const auto DEFAULT_PROGRESS_PRINTER = defaultProgressPrinter<mc::INFO, 30>;

    template<typename Rep, typename Period>
    [[nodiscard]]
    auto watchProgress(mc::Logger &logger, const Progress &progress,
                       const std::chrono::duration<Rep, Period> &updateInterval,
                       const ProgressPrinter &progressPrinter = DEFAULT_PROGRESS_PRINTER) -> std::thread {
        return std::thread([&, progressPrinter, updateInterval] {
            while (true) {
                if (progress.total == 0) {
                    std::this_thread::sleep_for(updateInterval);
                    continue;
                }
                logger.stream() << '\r';
                progressPrinter(logger, progress);
                logger.stream() << "\033[K" << std::flush;

                if (progress.completed()) {
                    logger.stream() << std::endl;
                    return;
                }
                std::this_thread::sleep_for(updateInterval);
            }
        });
    }

    [[nodiscard]]
    auto watchProgress(mc::Logger &logger, const Progress &progress) -> std::thread;
}
