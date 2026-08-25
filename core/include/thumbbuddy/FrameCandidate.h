//
// Created by Robert Harrison on 8/25/26.
//

#ifndef THUMBBUDDY_FRAMECANDIDATE_H
#define THUMBBUDDY_FRAMECANDIDATE_H

namespace thumbbuddy {
    struct FrameCandidate {
        double timestamp {};
        double sharpness {};
        double brightness {};
        double motion {};
    };
}

#endif //THUMBBUDDY_FRAMECANDIDATE_H
