//
// Created by Robert Harrison on 9/24/26.
//

#include <Format.h>

#include <cmath>
#include <format>

namespace thumbbuddy::cli {
    std::string formatDuration(double seconds) {
        const long long total = std::llround(seconds);
        const long long hours = total / 3600;
        const long long minutes = (total / 60) % 60;
        const long long secs = total % 60;

        if (hours > 0) {
            return std::format("{}:{:02}:{:02}", hours, minutes, secs);
        }
        return std::format("{}:{:02}", minutes, secs);
    }

    std::string toMbps(std::uint64_t bitsPerSecond) {
        const double mbps = static_cast<double>(bitsPerSecond) / 1'000'000.0;
        return std::format("{:.1f} Mbps", mbps);
    }
}
