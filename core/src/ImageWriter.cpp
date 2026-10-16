//
// Created by Robert Harrison on 10/15/26.
//

#include <thumbbuddy/ImageWriter.h>
#include <cstdint>
#include <fstream>
#include <system_error>
#include "ffmpeg/Wrappers.h"

namespace thumbbuddy {
    bool ImageWriter::writePNG(const ImageBuffer &image, const std::filesystem::path &path, std::string *error) {
        // Verify image is valid.
        if (image.width == 0 || image.height == 0) {
            *error = "image width and height must be non-zero";
            return false;
        }

        if (image.pixelFormat != PixelFormat::RGBA8) {
            *error = "pixelFormat must be RGBA8";
            return false;
        }

        if (image.stride < image.width * 4) {
            *error = "image stride must be >= width * 4";
            return false;
        }

        if (image.pixels.size() < static_cast<std::size_t>(image.stride) * image.height) {
            *error = "pixel buffer is smaller than stride x height";
            return false;
        }

        // Find the encoder.
        const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_PNG);
        if (!codec) {
            *error = "could not find encoder for PNG codec";
            return false;
        }

        // Setup encoder context.
        ffmpeg::CodecContextPtr encoderContext { avcodec_alloc_context3(codec) };
        if (!encoderContext) {
            *error = "out of memory";
            return false;
        }

        encoderContext->width = static_cast<int>(image.width);
        encoderContext->height = static_cast<int>(image.height);
        encoderContext->pix_fmt = AV_PIX_FMT_RGBA;
        encoderContext->time_base = {1, 1}; // avcodec_open2 fails without this.

        const int openResult = avcodec_open2(encoderContext.get(), codec, nullptr);
        if (openResult < 0) {
            *error = ffmpeg::describeError(openResult);
            return false;
        }

        // Setup AVFrame.
        ffmpeg::FramePtr frame { av_frame_alloc() };
        if (!frame) {
            *error = "out of memory";
            return false;
        }
        frame->format = AV_PIX_FMT_RGBA;
        frame->width = static_cast<int>(image.width);
        frame->height = static_cast<int>(image.height);
        frame->data[0] = const_cast<std::uint8_t *>(image.pixels.data()); // Safe to cast because the encoder only reads the data.
        frame->linesize[0] = static_cast<int>(image.stride);

        // Send frame to the encoder.
        const int sendResult = avcodec_send_frame(encoderContext.get(), frame.get());
        if (sendResult < 0) {
            *error = ffmpeg::describeError(sendResult);
            return false;
        }

        // Flush. No more frames to send.
        const int flushResult = avcodec_send_frame(encoderContext.get(), nullptr);
        if (flushResult < 0) {
            *error = ffmpeg::describeError(flushResult);
            return false;
        }

        // Read encoded packets.
        ffmpeg::PacketPtr packet { av_packet_alloc() };
        if (!packet) {
            *error = "out of memory";
            return false;
        }

        // Open output stream.
        std::ofstream outputFile(path, std::ios::binary);
        if (!outputFile.is_open()) {
            *error = "could not open " + path.string() + " for writing";
            return false;
        }

        int readResult;
        while ((readResult = avcodec_receive_packet(encoderContext.get(), packet.get())) == 0) {
            // Write packet data to file.
            outputFile.write(reinterpret_cast<const char *>(packet->data), packet->size);

            // Clear the packet data references before the next iteration.
            av_packet_unref(packet.get());
        }

        outputFile.close();

        if (readResult != AVERROR_EOF) {
            *error = ffmpeg::describeError(readResult);

            // Clean up partial file (if any).
            std::error_code ec;
            if (!std::filesystem::remove(path, ec) && ec) {
                *error += " (partial file remaining: " + ec.message() + ")";
            }

            return false;
        }

        if (outputFile.fail()) {
            *error = "could not write " + path.string();

            // Clean up partial file (if any).
            std::error_code ec;
            if (!std::filesystem::remove(path, ec) && ec) {
                *error += " (partial file remaining: " + ec.message() + ")";
            }

            return false;
        }

        return true;
    }
}