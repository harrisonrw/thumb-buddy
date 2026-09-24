//
// Created by Robert Harrison on 8/23/26.
//

#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <thumbbuddy/FrameCandidate.h>
#include <thumbbuddy/Version.h>
#include <thumbbuddy/MediaReader.h>
#include <Options.h>
#include <Format.h>

static int showHelp() {
    std::cout << "Usage: thumbbuddy [options]" << "\n";
    std::cout << "\n";
    std::cout << "Options:" << "\n";
    std::cout << "-h, --help             Print this help" << "\n";
    std::cout << "-v, --version          Output the version number" << "\n";
    std::cout << "-i, --info <path>      Output metadata for file" << "\n";

    return 0;
}

static int showVersion() {
    std::cout << "thumbbuddy version " << thumbbuddy::kVersion << "\n";
    return 0;
}

static int showInfo(const std::vector<std::string_view>& inputs) {
    if (inputs.empty()) {
        std::cerr << "thumbbuddy: --info requires a path" << "\n";
        return 2;
    }

    const std::string path { inputs.front() };

    std::string error;
    const std::optional<thumbbuddy::MediaReader> reader = thumbbuddy::MediaReader::open(path, &error);
    if (!reader.has_value()) {
        std::cerr << "thumbbuddy: " << path << ": " << error << "\n";
        return 1;
    }

    const thumbbuddy::MediaInfo info = reader->info();
    if (info.duration.has_value()) {
        std::cout << "Duration: " << thumbbuddy::cli::formatDuration(info.duration.value()) << "\n";
    }

    if (info.mediaType != thumbbuddy::MediaType::unknown) {
        std::cout << "Resolution: " << info.width << "x" << info.height << "\n";
    }

    if (info.frameRate.has_value()) {
        std::cout << "Frame Rate: " << info.frameRate.value().toString() << "\n";
    }

    if (info.bitRate.has_value()) {
        std::cout << "Bit Rate: " << thumbbuddy::cli::toMbps(info.bitRate.value()) << "\n";
    }

    if (info.codec.has_value()) {
        std::cout << "Codec: " << info.codec.value() << "\n";
    }

    return 0;
}

int main(int argc, char* argv[]) {
    const std::span<char* const> args = std::span<char * const> { argv + 1, static_cast<std::size_t>(argc > 0 ? argc - 1: 0) };
    const thumbbuddy::cli::Options options = thumbbuddy::cli::parseArgs(args);

    if (options.error.has_value()) {
        std::cerr << "thumbbuddy: " << options.error.value() << "\n";
        return 2;
    }

    switch (options.mode) {
        case thumbbuddy::cli::Mode::help:
            return showHelp();
        case thumbbuddy::cli::Mode::version:
            return showVersion();
        case thumbbuddy::cli::Mode::info:
            return showInfo(options.inputs);
        case thumbbuddy::cli::Mode::run:
            break;
    }

    std::cout << "Thumb Buddy\n";

    thumbbuddy::FrameCandidate frameCandidate = {  0.0, 0.20, 0.50, 0.10 };

    return 0;
}
