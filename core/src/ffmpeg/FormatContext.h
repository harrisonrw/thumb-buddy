//
// Created by Robert Harrison on 10/12/26.
//

#ifndef THUMBBUDDY_FFMPEG_FORMATCONTEXT_H
#define THUMBBUDDY_FFMPEG_FORMATCONTEXT_H

#include <filesystem>
#include <memory>
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace thumbbuddy::ffmpeg {
    struct FormatContextDeleter {
        void operator()(AVFormatContext *ctx) const {
            avformat_close_input(&ctx);
        };
    };

    using FormatContextPtr = std::unique_ptr<AVFormatContext, FormatContextDeleter>;

    [[nodiscard]] FormatContextPtr openInput(const std::filesystem::path &path, std::string *error);

    [[nodiscard]] std::string describeError(int errorCode);
}

#endif //THUMBBUDDY_FFMPEG_FORMATCONTEXT_H
