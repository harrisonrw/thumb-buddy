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
        found,      // Frame now holds the target frame.
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

        // Some codecs decode to RGB (PNG-in-MOV, QuickTime Animation, some ProRes 4444), which has no YUV matrix or
        // limited range, so the defaults only apply to YUV.
        static void applyColorDefaults(AVFrame *frame) {
            const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(frame->format));
            if (descriptor && (descriptor->flags & AV_PIX_FMT_FLAG_RGB)) {
                // is RGB.
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
            // Compute the timestamp (in the frame's own time).
            AVStream *stream = formatContext->streams[streamIndex];
            std::int64_t timestamp = decoded->best_effort_timestamp;
            if (timestamp == AV_NOPTS_VALUE) {
                // best effort is unknown, so fallback to pts.
                timestamp = decoded->pts;
            }

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

        ReceiveResult receiveUntil(AVFrame *frame, AVFrame *lastFrame, std::int64_t target, std::string *error) {
            int result;
            while ((result = avcodec_receive_frame(decoderContext.get(), frame)) == 0) {
                if (frame->best_effort_timestamp < target) {
                    // Before the target. So keep it, since it is the frame on screen if the next one is past the target.
                    av_frame_unref(lastFrame);
                    av_frame_move_ref(lastFrame, frame);
                    continue;
                }
                if (frame->best_effort_timestamp > target && lastFrame->buf[0] != nullptr) {
                    // Past the target. So use the last frame.
                    av_frame_unref(frame);
                    av_frame_move_ref(frame, lastFrame);
                }
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

            // Convert seconds into stream timebase units.
            std::int64_t target = std::llround(seconds / av_q2d(stream->time_base));
            if (stream->start_time != AV_NOPTS_VALUE) {
                target += stream->start_time;
            }

            // Seek.
            int seekResult = av_seek_frame(formatContext.get(), streamIndex, target, AVSEEK_FLAG_BACKWARD);
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
                    receiveResult = receiveUntil(frame.get(), lastFrame.get(), target, error);

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
                receiveResult = receiveUntil(frame.get(), lastFrame.get(), target, error);
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

        // Conversion (including 10-bit to 8-bit) must be the same on every platform. By default swscale picks a SIMD
        // path per CPU, and the paths can round differently, so macOS (ARM) and Linux (x86) could produce different
        // pixels and different scores. BITEXACT makes every platform produce identical output.
        // Not ACCURATE_RND: it switches to a different conversion path that changes color edges by up to 131 levels.
        scalerContext->flags |= SWS_BITEXACT;

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