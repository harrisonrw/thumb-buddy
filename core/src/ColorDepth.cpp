//
// Created by Robert Harrison on 9/24/26.
//

#include <thumbbuddy/ColorDepth.h>

extern "C" {
#include <libavutil/pixdesc.h>
}

namespace thumbbuddy {
    std::optional<int> colorDepthFor(const int format, const int bitsPerRawSample) {
        const AVPixFmtDescriptor *descriptor = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(format));

        if (descriptor == nullptr) {
            // Some codecs report a sample depth without a pixel format.
            return bitsPerRawSample > 0
                ? std::optional { bitsPerRawSample }
                : std::nullopt;
        }

        return descriptor->comp[0].depth;
    }
}
