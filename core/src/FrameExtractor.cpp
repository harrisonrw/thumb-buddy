//
// Created by Robert Harrison on 10/9/26.
//

#include <thumbbuddy/FrameExtractor.h>
#include "ffmpeg/FormatContext.h"

namespace thumbbuddy {
    struct FrameExtractor::Impl {
        ffmpeg::FormatContextPtr formatContext;
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

        auto impl = std::make_unique<Impl>();
        impl->formatContext = std::move(formatContext);
        return FrameExtractor { std::move(impl) };
    }

    std::vector<Frame> FrameExtractor::extractFrames() const {
        // TODO: implement frame extraction.
        return std::vector<Frame> {};
    }
} // thumbbuddy