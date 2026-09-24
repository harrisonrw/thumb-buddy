//
// Created by Robert Harrison on 9/24/26.
//

#ifndef THUMBBUDDY_LOGGING_H
#define THUMBBUDDY_LOGGING_H

namespace thumbbuddy {
    enum class LogLevel {
        quiet = 0,
        error,
        warning,
        info,
        debug
    };

    void setLogLevel(LogLevel level);
}

#endif //THUMBBUDDY_LOGGING_H
