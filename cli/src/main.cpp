//
// Created by Robert Harrison on 8/23/26.
//

#include <iostream>
#include <span>
#include <string_view>
#include <thumbbuddy/FrameCandidate.h>
#include <thumbbuddy/Version.h>
#include "Options.h"

int main(int argc, char* argv[]) {
    const std::span<char* const> args = std::span<char * const> { argv + 1, static_cast<std::size_t>(argc > 0 ? argc - 1: 0) };
    const thumbbuddy::cli::Options options = thumbbuddy::cli::parseArgs(args);

    if (options.error.has_value()) {
        std::cerr << options.error.value() << "\n";
        return 1;
    }

    switch (options.mode) {
        case thumbbuddy::cli::Mode::help:
            break;
        case thumbbuddy::cli::Mode::version:
            std::cout << "thumbbuddy version " << thumbbuddy::kVersion << "\n";
            return 0;
        case thumbbuddy::cli::Mode::run:
            break;
    }

    std::cout << "Thumb Buddy\n";

    thumbbuddy::FrameCandidate frameCandidate = {  0.0, 0.20, 0.50, 0.10 };

    return 0;
}
