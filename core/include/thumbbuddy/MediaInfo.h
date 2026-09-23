//
// Created by Robert Harrison on 9/1/26.
//

#ifndef THUMBBUDDY_MEDIAINFO_H
#define THUMBBUDDY_MEDIAINFO_H

#include <string>
#include <optional>

#include "FrameRate.h"
#include "MediaType.h"

namespace thumbbuddy {
    struct MediaInfo {
        MediaType mediaType {};
        int width {};
        int height {};
        std::optional<double> duration {}; // in seconds
        std::optional<FrameRate> frameRate {};
        std::optional<std::string> codec {};
        std::optional<std::uint64_t> bitRate {}; // in bits per second
    };
}

#endif //THUMBBUDDY_MEDIAINFO_H
