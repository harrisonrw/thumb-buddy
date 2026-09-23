//
// Created by Robert Harrison on 9/15/26.
//

#include <string_view>

#include <thumbbuddy/MediaReader.h>
#include <thumbbuddy/FrameRate.h>
#include <thumbbuddy/MediaInfo.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/codec_id.h>
}

namespace {
    std::string describeError(int errorCode) {
        char buffer[AV_ERROR_MAX_STRING_SIZE] {};
        av_strerror(errorCode, buffer, sizeof buffer);
        return buffer;
    }
}

namespace thumbbuddy {
    MediaReader::MediaReader(AVFormatContext *ctx) noexcept : formatContext_ { ctx } {}

    std::optional<MediaReader> MediaReader::open(const std::filesystem::path &path,
                                                 std::string *error) {
        AVFormatContext *ctx = nullptr;

        const int openResult = avformat_open_input(&ctx, path.c_str(), nullptr, nullptr);
        if (openResult < 0) {
            *error = describeError(openResult);
            return std::nullopt;
        }

        const int infoResult = avformat_find_stream_info(ctx, nullptr);
        if (infoResult < 0) {
            avformat_close_input(&ctx);
            *error = describeError(infoResult);
            return std::nullopt;
        }

        return MediaReader { ctx };
    }

    MediaInfo MediaReader::info() const {
        MediaInfo mediaInfo = {};
        mediaInfo.duration = containerDuration();

        const int index = av_find_best_stream(formatContext_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (index < 0) {
            // No video stream.
            return mediaInfo;
        }

        AVStream *stream = formatContext_->streams[index];
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

        if (isStillImage()) {
            mediaInfo.mediaType = MediaType::image;
            mediaInfo.duration = std::nullopt; // ffmpeg gives images a one-frame timeline
        } else {
            mediaInfo.mediaType = MediaType::video;
            mediaInfo.frameRate = guessFrameRate(stream);

            if (formatContext_.get()->bit_rate > 0) {
                mediaInfo.bitRate = static_cast<std::uint64_t>(formatContext_.get()->bit_rate);
            }
        }

        return mediaInfo;
    }

    void MediaReader::AVFormatContextDeleter::operator()(AVFormatContext *ctx) const noexcept {
        avformat_close_input(&ctx);
    }

    std::optional<double> MediaReader::containerDuration() const {
        if (formatContext_->duration == AV_NOPTS_VALUE) {
            return std::nullopt;
        }
        return static_cast<double>(formatContext_->duration) / AV_TIME_BASE;  // micro-seconds to seconds
    }

    std::optional<FrameRate> MediaReader::guessFrameRate(AVStream *stream) const {
        const AVRational frameRate = av_guess_frame_rate(formatContext_.get(), stream, nullptr);
        if (frameRate.num <= 0 || frameRate.den <= 0) {
            // av_guess_frame_rate returns 0/1 when it has no idea
            return std::nullopt;
        }
        return FrameRate { frameRate.num, frameRate.den };
    }

    bool MediaReader::isStillImage() const {
        // ffmpeg models a still image as a single-frame video stream.
        const std::string_view demuxer = formatContext_->iformat->name;
        return demuxer == "image2" || demuxer.ends_with("_pipe");
    }
} // thumbbuddy
