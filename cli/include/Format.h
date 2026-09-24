//
// Created by Robert Harrison on 9/24/26.
//

#ifndef THUMBBUDDY_CLI_FORMAT_H
#define THUMBBUDDY_CLI_FORMAT_H

#include <cstdint>
#include <string>

namespace thumbbuddy::cli {
    [[nodiscard]] std::string formatDuration(double seconds);
    [[nodiscard]] std::string toMbps(std::uint64_t bitsPerSecond);
}

#endif //THUMBBUDDY_CLI_FORMAT_H
