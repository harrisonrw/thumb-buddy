//
// Created by Robert Harrison on 8/23/26.
//

#include <iostream>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <format>
#include <thumbbuddy/FrameCandidate.h>
#include <thumbbuddy/Version.h>
#include <thumbbuddy/FrameExtractor.h>
#include <thumbbuddy/Frame.h>
#include <thumbbuddy/ImageWriter.h>
#include <thumbbuddy/MediaReader.h>
#include <thumbbuddy/Logging.h>
#include <Options.h>
#include <Format.h>

static int showHelp() {
    std::cout << "Usage: thumbbuddy [options]" << "\n";
    std::cout << "\n";
    std::cout << "Options:" << "\n";
    std::cout << "-h, --help             Print this help" << "\n";
    std::cout << "-v, --version          Output the version number" << "\n";
    std::cout << "-i, --info <path>      Output metadata for file" << "\n";
    std::cout << "-o, --output <dir>     Directory for PNGs (default: same as video)" << "\n";

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

    if (info.colorDepth.has_value()) {
        std::cout << "Color Depth: " << info.colorDepth.value() << "-bit" << "\n";
    }

    if (info.bitRate.has_value()) {
        std::cout << "Bit Rate: " << thumbbuddy::cli::toMbps(info.bitRate.value()) << "\n";
    }

    if (info.codec.has_value()) {
        std::cout << "Codec: " << info.codec.value() << "\n";
    }

    return 0;
}

static int extractFrames(const std::vector<std::string_view>& inputs, const std::optional<std::string>& outputDirectory) {
    if (inputs.empty()) {
        std::cerr << "thumbbuddy: requires a path" << "\n";
        return 2;
    }

    const std::string path { inputs.front() };
    const std::filesystem::path inputPath { path };               // "/Users/robert/video/skiing.mp4"

    std::filesystem::path directory = outputDirectory
        ? std::filesystem::path { outputDirectory.value() }  // "/Users/robert/some_directory"
        : inputPath.parent_path();                               // "/Users/robert/video"

    const std::string stem = inputPath.stem().string(); // "skiing"

    // Convert an empty path into the current directory.
    if (directory.empty()) {
        directory = ".";
    }

    std::error_code ec;
    if (!std::filesystem::is_directory(directory, ec)) {
        std::cerr << "thumbbuddy: " << directory.string() << ": not a directory" << "\n";
        return 1;
    }

    std::string error;
    std::optional<thumbbuddy::FrameExtractor> frameExtractor = thumbbuddy::FrameExtractor::open(path, &error);
    if (!frameExtractor.has_value()) {
        std::cerr << "thumbbuddy: " << path << ": " << error << "\n";
        return 1;
    }

    std::optional<std::vector<thumbbuddy::Frame>> frames = frameExtractor->extractFrames(5.0, &error);
    if (!frames.has_value()) {
        std::cerr << "thumbbuddy: " << path << ": " << error << "\n";
        return 1;
    }

    std::cout << "Extracted Frames: " << frames->size() << "\n";

    const std::size_t numDigits = std::to_string(frames->size()).size(); // 12 frames -> 2 digits

    int writeResult = 0;
    for (std::size_t index = 1; const thumbbuddy::Frame &frame : *frames) {
        std::cout << "Frame : " << thumbbuddy::cli::formatDuration(frame.timestamp) << "\n";
        
        const std::string filename = std::format("{}-{:0{}}.png", stem, index, numDigits); // "skiing-01.png"
        const std::filesystem::path outputPath = directory / filename;

        std::string writeError;
        if (!thumbbuddy::ImageWriter::writePNG(frame.image, outputPath, &writeError)) {
            std::cerr << "thumbbuddy: " << outputPath.string() << ": " << writeError << "\n";
            writeResult = 1;
        }

        index++;
    }

    return writeResult;
}

int main(int argc, char* argv[]) {
    // TODO: Suppress info and warning messages from FFmpeg by setting to 'error' in Release builds.
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::info);

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
            return extractFrames(options.inputs, options.outputDirectory);
    }

    return 0;
}
