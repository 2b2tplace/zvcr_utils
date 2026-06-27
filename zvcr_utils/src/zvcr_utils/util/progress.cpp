#include <zvcr_utils/util/progress.hpp>

namespace zvcr {
    auto Progress::increment() -> void {
        if (current == 0)
            startedTimestamp = std::chrono::steady_clock::now();
        ++current;
    }

    auto Progress::percentage() const -> float {
        if (total == 0) return 1.0f;
        if (current == 0) return 0.0f;
        return static_cast<float>(current) / static_cast<float>(total);
    }

    auto Progress::completed() const -> bool {
        return total == 0 || (current > 0 && current >= total);
    }

    auto Progress::elapsed() const -> std::chrono::steady_clock::duration {
        return std::chrono::steady_clock::now() - startedTimestamp;
    }

    auto formatDuration(std::chrono::seconds secs) -> std::string {
        const auto h = std::chrono::duration_cast<std::chrono::hours>(secs);
        secs -= h;

        const auto m = std::chrono::duration_cast<std::chrono::minutes>(secs);
        secs -= m;

        std::ostringstream oss;
        if (h.count() > 0)
            oss << h.count() << "h ";
        if (m.count() > 0 || h.count() > 0)
            oss << m.count() << "m ";
        oss << secs.count() << "s";

        return oss.str();
    }

    auto formatRemainingETA(const Progress &progress) -> std::string {
        if (progress.current == 0 || progress.total == 0) return "?";

        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(progress.elapsed());
        const double percentage = progress.percentage();

        if (percentage <= 0.0) return "?";

        const auto estimatedTotalMillis = elapsed.count() / percentage;
        const auto remainingMillis = static_cast<int64_t>(estimatedTotalMillis - elapsed.count());
        if (remainingMillis < 0) return "?";

        return formatDuration(std::chrono::seconds(remainingMillis / 1000LL));
    }

    auto watchProgress(mc::Logger &logger, const Progress &progress) -> std::thread {
        return watchProgress(logger, progress, std::chrono::milliseconds(200L));
    }
}
