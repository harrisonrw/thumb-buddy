//
// Created by Robert Harrison on 9/15/26.
//

#ifndef THUMBBUDDY_MEDIAREADER_H
#define THUMBBUDDY_MEDIAREADER_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thumbbuddy/MediaInfo.h>

struct AVFormatContext;
struct AVStream;

namespace thumbbuddy {
    class MediaReader {
    public:
        [[nodiscard]] static std::optional<MediaReader> open(const std::filesystem::path &path,
                                                             std::string *error);

        [[nodiscard]] MediaInfo info() const;

    private:
        struct AVFormatContextDeleter {
            void operator()(AVFormatContext *ctx) const noexcept;
        };

        // Private constructor so a VideoReader can only come from open()
        explicit MediaReader(AVFormatContext *ctx) noexcept;

        [[nodiscard]] std::optional<double> containerDuration() const;
        [[nodiscard]] std::optional<FrameRate> guessFrameRate(AVStream *stream) const;
        [[nodiscard]] bool isStillImage() const;

        std::unique_ptr<AVFormatContext, AVFormatContextDeleter> formatContext_;
    };
}

#endif //THUMBBUDDY_MEDIAREADER_H
