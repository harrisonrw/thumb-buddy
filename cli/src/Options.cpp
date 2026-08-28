//
// Created by Robert Harrison on 8/28/26.
//

#include "Options.h"

namespace thumbbuddy::cli {
    Options parseArgs(std::span<char* const> args) {
        Options options {};

        for (std::size_t i = 0; i < args.size(); i++) {
            const std::string_view arg { args[i] };

            // Everything after "--" is a positional argument, even if it looks like a flag.
            if (arg == "--") {
                for (std::size_t j = i + 1; j < args.size(); j++) {
                    options.inputs.emplace_back(args[j]);
                }
                break;
            }

            if (arg == "--help" || arg == "-h") {
                options.mode = Mode::help;
            } else if (arg == "--version" || arg == "-v") {
                options.mode = Mode::version;
            } else if (arg.size() > 1 && arg.starts_with('-')) {
                // A lone "-" conventionally means stdin, so only longer tokens are flags.
                options.error = "unrecognized option '" + std::string { arg } + "'";
            } else {
                options.inputs.emplace_back(arg);
                options.mode = Mode::run;
            }
        }

        return options;
    }
}
