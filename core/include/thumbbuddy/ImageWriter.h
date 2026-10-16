//
// Created by Robert Harrison on 10/15/26.
//

#ifndef THUMBBUDDY_IMAGEWRITER_H
#define THUMBBUDDY_IMAGEWRITER_H

#include <filesystem>
#include <string>
#include <thumbbuddy/ImageBuffer.h>

namespace thumbbuddy {
    class ImageWriter {
    public:
        [[nodiscard]] static bool writePNG(const ImageBuffer &image, const std::filesystem::path &path, std::string *error);
    };
}

#endif //THUMBBUDDY_IMAGEWRITER_H
