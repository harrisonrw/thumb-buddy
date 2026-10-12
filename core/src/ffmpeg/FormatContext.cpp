//
// Created by Robert Harrison on 10/12/26.
//

#include "FormatContext.h"

namespace thumbbuddy::ffmpeg {

    FormatContextPtr openInput(const std::filesystem::path &path, std::string *error) {

        AVFormatContext *ctx = nullptr;

        const int openResult = avformat_open_input(&ctx, path.c_str(), nullptr, nullptr);
        if (openResult < 0) {
            *error = describeError(openResult);
            return nullptr;
        }

        const int infoResult = avformat_find_stream_info(ctx, nullptr);
        if (infoResult < 0) {
            avformat_close_input(&ctx);
            *error = describeError(infoResult);
            return nullptr;
        }

        return FormatContextPtr { ctx };
    }

    std::string describeError(int errorCode) {
        char buffer[AV_ERROR_MAX_STRING_SIZE] {};
        av_strerror(errorCode, buffer, sizeof buffer);
        return buffer;
    }
}