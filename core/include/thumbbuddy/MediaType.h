//
// Created by Robert Harrison on 9/1/26.
//

#ifndef THUMBBUDDY_MEDIATYPE_H
#define THUMBBUDDY_MEDIATYPE_H

#include <ostream>

namespace thumbbuddy {
    enum class MediaType {
        unknown = 0,
        image,
        video
    };

    inline std::ostream& operator<<(std::ostream& os, MediaType mediaType) {
        switch (mediaType) {
            case MediaType::unknown: return os << "unknown";
            case MediaType::image:   return os << "image";
            case MediaType::video:   return os << "video";
        }
        return os << "invalid";
    }
}

#endif //THUMBBUDDY_MEDIATYPE_H
