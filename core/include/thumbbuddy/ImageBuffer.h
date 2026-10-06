//
// Created by Robert Harrison on 10/6/26.
//

#ifndef THUMBBUDDY_IMAGEBUFFER_H
#define THUMBBUDDY_IMAGEBUFFER_H
#include <cstdint>
#include <vector>

namespace thumbbuddy {
    enum class PixelFormat {
        RGBA8
    };

    struct ImageBuffer {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t stride;
        PixelFormat pixelFormat;
        std::vector<std::uint8_t> pixels;
    };
}
#endif //THUMBBUDDY_IMAGEBUFFER_H
