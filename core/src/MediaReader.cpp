//
// Created by Robert Harrison on 9/15/26.
//

#include <string_view>

#include <thumbbuddy/MediaReader.h>
#include "ffmpeg/FormatContext.h"
#include <thumbbuddy/FrameRate.h>
#include <thumbbuddy/MediaInfo.h>
#include <thumbbuddy/ColorDepth.h>

extern "C" {
#include <libavcodec/codec_id.h>
}

namespace thumbbuddy {
    struct MediaReader::Impl {
        ffmpeg::FormatContextPtr formatContext;

        [[nodiscard]] std::optional<double> containerDuration() const {
            if (formatContext->duration == AV_NOPTS_VALUE) {
                return std::nullopt;
            }
            return static_cast<double>(formatContext->duration) / AV_TIME_BASE;  // micro-seconds to seconds
        }

        [[nodiscard]] std::optional<FrameRate> guessFrameRate(AVStream *stream) const {
            const AVRational frameRate = av_guess_frame_rate(formatContext.get(), stream, nullptr);
            if (frameRate.num <= 0 || frameRate.den <= 0) {
                // av_guess_frame_rate returns 0/1 when it has no idea
                return std::nullopt;
            }
            return FrameRate { frameRate.num, frameRate.den };
        }

        [[nodiscard]] bool isStillImage() const {
            // ffmpeg models a still image as a single-frame video stream.
            const std::string_view demuxer = formatContext->iformat->name;
            return demuxer == "image2" || demuxer.ends_with("_pipe");
        }
    };

    MediaReader::MediaReader(std::unique_ptr<Impl> impl) noexcept : impl_ { std::move(impl) } {}
    MediaReader::~MediaReader() = default;
    MediaReader::MediaReader(MediaReader &&) noexcept = default;
    MediaReader &MediaReader::operator=(MediaReader &&) noexcept = default;

    std::optional<MediaReader> MediaReader::open(const std::filesystem::path &path,
                                                 std::string *error) {
        ffmpeg::FormatContextPtr formatContext = ffmpeg::openInput(path, error);
        if (!formatContext) {
            return std::nullopt;
        }

        auto impl = std::make_unique<Impl>();
        impl->formatContext = std::move(formatContext);
        return MediaReader { std::move(impl) };
    }

    MediaInfo MediaReader::info() const {
        MediaInfo mediaInfo = {};
        mediaInfo.duration = impl_->containerDuration();

        const int index = av_find_best_stream(impl_->formatContext.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (index < 0) {
            // No video stream.
            return mediaInfo;
        }

        AVStream *stream = impl_->formatContext->streams[index];
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            // Cover art in an audio file.
            return mediaInfo;
        }

        const AVCodecParameters *parameters = stream->codecpar;
        mediaInfo.width = parameters->width;
        mediaInfo.height = parameters->height;

        if (parameters->codec_id != AV_CODEC_ID_NONE) {
            mediaInfo.codec = avcodec_get_name(parameters->codec_id);
        }

        if (impl_->isStillImage()) {
            mediaInfo.mediaType = MediaType::image;
            mediaInfo.duration = std::nullopt; // ffmpeg gives images a one-frame timeline
        } else {
            mediaInfo.mediaType = MediaType::video;
            mediaInfo.frameRate = impl_->guessFrameRate(stream);

            if (impl_->formatContext->bit_rate > 0) {
                mediaInfo.bitRate = static_cast<std::uint64_t>(impl_->formatContext->bit_rate);
            }
        }

        mediaInfo.colorDepth = colorDepthFor(parameters->format, parameters->bits_per_raw_sample);

        return mediaInfo;
    }
} // thumbbuddy
