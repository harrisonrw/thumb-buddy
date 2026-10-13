//
// Created by Robert Harrison on 10/9/26.
//

#include <thumbbuddy/FrameExtractor.h>
#include "ffmpeg/Wrappers.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace thumbbuddy {
    struct FrameExtractor::Impl {
        ffmpeg::FormatContextPtr formatContext;
        ffmpeg::CodecContextPtr decoderContext;
        int streamIndex {};
    };

    FrameExtractor::FrameExtractor(std::unique_ptr<Impl> impl) noexcept : impl_ { std::move(impl) } {}
    FrameExtractor::~FrameExtractor() = default;
    FrameExtractor::FrameExtractor(FrameExtractor &&) noexcept = default;
    FrameExtractor &FrameExtractor::operator=(FrameExtractor &&) noexcept = default;

    std::optional<FrameExtractor> FrameExtractor::open(const std::filesystem::path &path, std::string *error) {
        ffmpeg::FormatContextPtr formatContext = ffmpeg::openInput(path, error);
        if (!formatContext) {
            return std::nullopt;
        }

        // Get the decoder and stream index.
        const AVCodec *decoder = nullptr;
        const int streamIndex = av_find_best_stream(formatContext.get(), AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
        if (streamIndex < 0) {
            // No video stream.
            *error = ffmpeg::describeError(streamIndex);
            return std::nullopt;
        }
        ffmpeg::CodecContextPtr decoderContext { avcodec_alloc_context3(decoder) };
        if (!decoderContext) {
            *error = "out of memory";
            return std::nullopt;
        }

        // Get the stream.
        AVStream *stream = formatContext->streams[streamIndex];
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            // Cover art in an audio file.
            *error = "no video stream";
            return std::nullopt;
        }

        // Open the decoder.
        const int codecParResult = avcodec_parameters_to_context(decoderContext.get(), stream->codecpar);
        if (codecParResult < 0) {
            *error = ffmpeg::describeError(codecParResult);
            return std::nullopt;
        }
        const int openResult = avcodec_open2(decoderContext.get(), decoderContext->codec, nullptr);
        if (openResult < 0) {
            *error = ffmpeg::describeError(openResult);
            return std::nullopt;
        }

        auto impl = std::make_unique<Impl>();
        impl->formatContext = std::move(formatContext);
        impl->streamIndex = streamIndex;
        impl->decoderContext = std::move(decoderContext);

        return FrameExtractor { std::move(impl) };
    }

    std::vector<Frame> FrameExtractor::extractFrames() const {
        // TODO: iterate and get frames
        return std::vector<Frame> {};
    }
} // thumbbuddy