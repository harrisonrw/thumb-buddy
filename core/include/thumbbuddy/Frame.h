//
// Created by Robert Harrison on 10/12/26.
//

#ifndef THUMBBUDDY_FRAME_H
#define THUMBBUDDY_FRAME_H
#include "ImageBuffer.h"

namespace thumbbuddy {
    struct Frame {
        double timestamp {};
        ImageBuffer image;
    };
}

#endif //THUMBBUDDY_FRAME_H
