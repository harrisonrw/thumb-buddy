//
// Created by Robert Harrison on 10/9/26.
//

#include <cstdint>
#include <cmath>
#include <utility>
#include <thumbbuddy/FrameExtractor.h>
#include "ffmpeg/Wrappers.h"

extern "C" {
    #include <libavutil/pixdesc.h>
}

namespace {
    enum class ReceiveResult {
        found,      // Frame now holds the frame on screen at the target (see receiveUntil).
        needsInput, // Decoder wants another packet (EAGAIN).
        finished,   // Decoder is fully drained (EOF).
        failed      // An actual error. `*error` is set.
    };
}

namespace thumbbuddy {
    struct FrameExtractor::Impl {
        ffmpeg::FormatContextPtr formatContext;
        ffmpeg::CodecContextPtr decoderContext;
        ffmpeg::ScalerContextPtr scalerContext;
        int streamIndex {};

        // The frame's presentation time in stream ticks.
        // best_effort_timestamp if provided by decoder, otherwise fallback to pts.
        static std::int64_t frameTicks(const AVFrame *frame) {
            if (frame->best_effort_timestamp != AV_NOPTS_VALUE) {
                return frame->best_effort_timestamp;
            }
            return frame->pts;
        }

        // Some codecs decode to RGB (PNG-in-MOV, QuickTime Animation, some ProRes 4444),
        // which has no YUV matrix or limited range, so the defaults only apply to YUV.
        static void applyColorDefaults(AVFrame *frame) {
            const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(frame->format));
            if (descriptor && (descriptor->flags & AV_PIX_FMT_FLAG_RGB)) {
                return;
            }

            // Fill in missing color space and range on the decoded frame.
            if (frame->colorspace == AVCOL_SPC_UNSPECIFIED) {
                frame->colorspace = frame->height >= 720 ? AVCOL_SPC_BT709 : AVCOL_SPC_SMPTE170M;
            }
            if (frame->color_range == AVCOL_RANGE_UNSPECIFIED) {
                frame->color_range = AVCOL_RANGE_MPEG;
            }
        }

        std::optional<Frame> convert(AVFrame *decoded, std::string *error) {
            // Compute the timestamp (in the frame's own time). receiveUntil has already rejected frames without one.
            AVStream *stream = formatContext->streams[streamIndex];
            std::int64_t timestamp = frameTicks(decoded);

            if (stream->start_time != AV_NOPTS_VALUE) {
                // start time known, so subtract it
                timestamp -= stream->start_time;
            }

            // Convert to seconds.
            double frameTimestamp = static_cast<double>(timestamp) * av_q2d(stream->time_base);

            // Create the Image Buffer.

            std::uint32_t stride = static_cast<std::uint32_t>(decoded->width) * 4; // 4 Bytes Per Pixel

            ImageBuffer imageBuffer {
                .width = static_cast<std::uint32_t>(decoded->width),
                .height = static_cast<std::uint32_t>(decoded->height),
                .stride = stride,
                .pixelFormat = PixelFormat::RGBA8
            };

            // Resize so the vector owns the memory before the scaler writes into it.
            imageBuffer.pixels.resize(stride * static_cast<std::uint32_t>(decoded->height));

            applyColorDefaults(decoded);

            // Setup destination AVFrame.
            ffmpeg::FramePtr destination { av_frame_alloc() };
            if (!destination) {
                *error = "out of memory";
                return std::nullopt;
            }
            destination->format = AV_PIX_FMT_RGBA;
            destination->color_range = AVCOL_RANGE_JPEG;
            destination->width = decoded->width;
            destination->height = decoded->height;
            destination->data[0] = imageBuffer.pixels.data();
            destination->linesize[0] = static_cast<int>(stride);

            // Perform pixel scaling.
            const int scaleResult = sws_scale_frame(scalerContext.get(), destination.get(), decoded);
            if (scaleResult < 0) {
                *error = ffmpeg::describeError(scaleResult);
                return std::nullopt;
            }

            return Frame {frameTimestamp, std::move(imageBuffer) };
        }

        // Receives decoded frames until it can return the frame on screen at targetTicks:
        //   - a frame exactly at the target is returned
        //   - otherwise the latest frame before the target is returned, once the next frame passes it
        //   - a target before the first frame returns the first frame
        // Frames before the target are kept in lastFrame, each replacing the previous one.
        ReceiveResult receiveUntil(AVFrame *frame, AVFrame *lastFrame, std::int64_t targetTicks, std::string *error) {
            int result;
            while ((result = avcodec_receive_frame(decoderContext.get(), frame)) == 0) {
                const std::int64_t ticks = frameTicks(frame);
                if (ticks == AV_NOPTS_VALUE) {
                    *error = "decoded frame has no timestamp";
                    return ReceiveResult::failed;
                }
                if (ticks < targetTicks) {
                    // Before the target: keep it, since it is the frame on screen if the next one is past the target.
                    av_frame_unref(lastFrame);
                    av_frame_move_ref(lastFrame, frame);
                    continue;
                }
                if (ticks > targetTicks && lastFrame->buf[0] != nullptr) {
                    // Past the target: we want the latest frame at or before the target, which is the kept one.
                    av_frame_unref(frame);
                    av_frame_move_ref(frame, lastFrame);
                }
                // Exactly on the target, swapped above, or past it with no earlier frame (target before the first frame).
                return ReceiveResult::found;
            }

            if (result == AVERROR(EAGAIN)) {
                return ReceiveResult::needsInput;
            }
            if (result == AVERROR_EOF) {
                return ReceiveResult::finished;
            }

            *error = ffmpeg::describeError(result);
            return ReceiveResult::failed;
        }

        std::optional<Frame> extractFrame(double seconds, std::string *error) {
            AVStream *stream = formatContext->streams[streamIndex];

            // Convert seconds into stream ticks (time_base units).
            std::int64_t targetTicks = std::llround(seconds / av_q2d(stream->time_base));
            if (stream->start_time != AV_NOPTS_VALUE) {
                // Offset by start_time, so 0 seconds is always the first frame, even if the stream starts later.
                targetTicks += stream->start_time;
            }

            // Seek back to the keyframe at or before the target.
            // Decoding has to start at a keyframe.
            // One seek per sample keeps the work per frame to one keyframe interval.
            int seekResult = av_seek_frame(formatContext.get(), streamIndex, targetTicks, AVSEEK_FLAG_BACKWARD);
            if (seekResult < 0) {
                *error = ffmpeg::describeError(seekResult);
                return std::nullopt;
            }
            // Flush decoder buffers so old frames don't bleed into new frame.
            avcodec_flush_buffers(decoderContext.get());

            ffmpeg::PacketPtr packet { av_packet_alloc() };
            ffmpeg::FramePtr frame { av_frame_alloc() };
            ffmpeg::FramePtr lastFrame { av_frame_alloc() };

            if (!packet || !frame || !lastFrame) {
                *error = "out of memory";
                return std::nullopt;
            }

            ReceiveResult receiveResult { ReceiveResult::needsInput };

            // The read loop.
            int readResult;
            while ((readResult = av_read_frame(formatContext.get(), packet.get())) >= 0) {
                // Verify the packet belongs to target video stream.
                if (packet->stream_index == streamIndex) {

                    // Send packet to decoder.
                    int result = avcodec_send_packet(decoderContext.get(), packet.get());
                    if (result < 0) {
                        *error = ffmpeg::describeError(result);
                        return std::nullopt;
                    }

                    // Receive decoded frame.
                    receiveResult = receiveUntil(frame.get(), lastFrame.get(), targetTicks, error);

                    if (receiveResult == ReceiveResult::found) {
                        return convert(frame.get(), error);
                    }
                    if (receiveResult == ReceiveResult::failed) {
                        return std::nullopt;
                    }
                }

                // Clear the packet data references before the next iteration.
                av_packet_unref(packet.get());
            }

            if (readResult != AVERROR_EOF) {
                *error = ffmpeg::describeError(readResult);
                return std::nullopt;
            }

            if (receiveResult == ReceiveResult::needsInput) {
                // Drain to get the final frame.
                avcodec_send_packet(decoderContext.get(), nullptr);
                receiveResult = receiveUntil(frame.get(), lastFrame.get(), targetTicks, error);
            }

            if (receiveResult == ReceiveResult::found) {
                return convert(frame.get(), error);
            }

            if (receiveResult == ReceiveResult::finished && lastFrame->buf[0] != nullptr) {
                return convert(lastFrame.get(), error);
            }

            if (receiveResult == ReceiveResult::failed) {
                return std::nullopt; // `*error` was already set by receiveUntil.
            }

            *error = "no frame at the requested time";
            return std::nullopt;
        }
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

        // Create an empty scaler context.
        ffmpeg::ScalerContextPtr scalerContext { sws_alloc_context() };
        if (!scalerContext) {
            *error = "out of memory";
            return std::nullopt;
        }

        // F6: conversion must be identical on every platform. For 8-bit 4:2:0 to RGB, swscale normally uses a
        // hand-written SSSE3 (x86) or NEON (ARM) converter, and those round differently. ACCURATE_RND turns
        // both off, so every CPU uses the general conversion path, and BITEXACT makes that path identical
        // everywhere. 10-bit input always takes the general path.
        scalerContext->flags |= SWS_BITEXACT | SWS_ACCURATE_RND;

        auto impl = std::make_unique<Impl>();
        impl->formatContext = std::move(formatContext);
        impl->streamIndex = streamIndex;
        impl->decoderContext = std::move(decoderContext);
        impl->scalerContext = std::move(scalerContext);

        return FrameExtractor { std::move(impl) };
    }

    std::optional<std::vector<Frame>> FrameExtractor::extractFrames(double interval, std::string *error) {
        if (interval <= 0.0) {
            *error = "interval must be greater than 0";
            return std::nullopt;
        }

        std::vector<Frame> frames = {};

        if (impl_->formatContext->duration == AV_NOPTS_VALUE) {
            // No duration, so fallback to frame at time 0
            std::optional<Frame> frame = impl_->extractFrame(0, error);
            if (!frame) {
                return std::nullopt; // `*error` already set by extractFrame.
            }

            frames.push_back(std::move(*frame));
            return frames;
        }

        double duration = static_cast<double>(impl_->formatContext->duration) / AV_TIME_BASE;

        int counter = 0;
        double seconds = 0.0;

        while (seconds < duration) {
            std::optional<Frame> frame = impl_->extractFrame(seconds, error);

            if (!frame) {
                return std::nullopt; // `*error` already set by extractFrame.
            }

            frames.push_back(std::move(*frame));

            // Uniform sampling.
            counter++;
            seconds = static_cast<double>(counter) * interval;
        }

        return frames;
    }


} // thumbbuddy