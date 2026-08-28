//
// Created by Robert Harrison on 8/26/26.
//

#ifndef THUMBBUDDY_CLI_OPTIONS_H
#define THUMBBUDDY_CLI_OPTIONS_H

#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace thumbbuddy::cli {
    enum class Mode {
        run,
        version,
        help
    };

    struct Options {
        Mode mode { Mode::run };
        std::vector<std::string_view> inputs {};
        std::optional<std::string> error {};
    };

    [[nodiscard]] Options parseArgs(std::span<char* const> args);
}

#endif //THUMBBUDDY_CLI_OPTIONS_H
