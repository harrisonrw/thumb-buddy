//
// Created by Robert Harrison on 10/9/26.
//

#ifndef THUMBBUDDY_FRAMEEXTRACTOR_H
#define THUMBBUDDY_FRAMEEXTRACTOR_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thumbbuddy/Frame.h>

namespace thumbbuddy {
    class FrameExtractor {
    public:
        [[nodiscard]] static std::optional<FrameExtractor> open(const std::filesystem::path &path, std::string *error);
        ~FrameExtractor();
        FrameExtractor(FrameExtractor &&) noexcept;
        FrameExtractor &operator=(FrameExtractor &&) noexcept;

        [[nodiscard]] std::vector<Frame> extractFrames() const;

    private:
        struct Impl;
        explicit FrameExtractor(std::unique_ptr<Impl> impl) noexcept;
        std::unique_ptr<Impl> impl_;
    };
}

#endif //THUMBBUDDY_FRAMEEXTRACTOR_H
