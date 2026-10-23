//
// Created by Robert Harrison on 10/9/26.
//

#ifndef THUMBBUDDY_FRAMEEXTRACTOR_H
#define THUMBBUDDY_FRAMEEXTRACTOR_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thumbbuddy/Frame.h>

namespace thumbbuddy {
    class FrameExtractor {
    public:
        [[nodiscard]] static std::optional<FrameExtractor> open(const std::filesystem::path &path, std::string *error);
        ~FrameExtractor();
        FrameExtractor(FrameExtractor &&) noexcept;
        FrameExtractor &operator=(FrameExtractor &&) noexcept;

        // Extracts one frame every `interval` seconds, starting at 0 and stopping before the end of the video.
        // For example, an interval of 5 on a 12-second video samples at 0, 5 and 10 seconds.
        [[nodiscard]] std::optional<std::vector<Frame>> extractFrames(double interval, std::string *error);

    private:
        struct Impl;
        explicit FrameExtractor(std::unique_ptr<Impl> impl) noexcept;
        std::unique_ptr<Impl> impl_;
    };
}

#endif //THUMBBUDDY_FRAMEEXTRACTOR_H
