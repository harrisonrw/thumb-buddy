//
// Created by Robert Harrison on 9/15/26.
//

#ifndef THUMBBUDDY_MEDIAREADER_H
#define THUMBBUDDY_MEDIAREADER_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thumbbuddy/MediaInfo.h>

namespace thumbbuddy {
    class MediaReader {
    public:
        [[nodiscard]] static std::optional<MediaReader> open(const std::filesystem::path &path,
                                                             std::string *error);

        ~MediaReader();
        MediaReader(MediaReader &&) noexcept;
        MediaReader &operator=(MediaReader &&) noexcept;

        [[nodiscard]] MediaInfo info() const;

    private:
        struct Impl;
        explicit MediaReader(std::unique_ptr<Impl> impl) noexcept;
        std::unique_ptr<Impl> impl_;
    };
}

#endif //THUMBBUDDY_MEDIAREADER_H
