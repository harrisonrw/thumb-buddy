//
// Created by Robert Harrison on 9/24/26.
//

#ifndef THUMBBUDDY_COLORDEPTH_H
#define THUMBBUDDY_COLORDEPTH_H

#include <optional>

namespace thumbbuddy {
    // Bits per component (ex 10 for a 10-bit video).
    // Takes the raw AVCodecParameters fields as ints so that callers and this header stay free of FFmpeg types.
    [[nodiscard]] std::optional<int> colorDepthFor(int format, int bitsPerRawSample);
}

#endif //THUMBBUDDY_COLORDEPTH_H
