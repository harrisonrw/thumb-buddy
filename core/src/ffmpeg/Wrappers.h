//
// Created by Robert Harrison on 10/12/26.
//

#ifndef THUMBBUDDY_FFMPEG_WRAPPERS_H
#define THUMBBUDDY_FFMPEG_WRAPPERS_H

#include <filesystem>
#include <memory>
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
}

namespace thumbbuddy::ffmpeg {
    struct FormatContextDeleter {
        void operator()(AVFormatContext *ctx) const {
            avformat_close_input(&ctx);
        };
    };

    struct CodecContextDeleter {
        void operator()(AVCodecContext *ctx) const {
            avcodec_free_context(&ctx);
        };
    };

    struct FrameDeleter {
        void operator()(AVFrame *frame) const {
            av_frame_free(&frame);
        }
    };

    struct PacketDeleter {
        void operator()(AVPacket *pkt) const {
            av_packet_free(&pkt);
        }
    };

    struct ScalerContextDeleter {
        void operator()(SwsContext *ctx) const {
            sws_free_context(&ctx);
        }
    };

    using FormatContextPtr = std::unique_ptr<AVFormatContext, FormatContextDeleter>;
    using CodecContextPtr = std::unique_ptr<AVCodecContext, CodecContextDeleter>;
    using FramePtr = std::unique_ptr<AVFrame, FrameDeleter>;
    using PacketPtr = std::unique_ptr<AVPacket, PacketDeleter>;
    using ScalerContextPtr = std::unique_ptr<SwsContext, ScalerContextDeleter>;

    [[nodiscard]] FormatContextPtr openInput(const std::filesystem::path &path, std::string *error);

    [[nodiscard]] std::string describeError(int errorCode);
}

#endif //THUMBBUDDY_FFMPEG_WRAPPERS_H
