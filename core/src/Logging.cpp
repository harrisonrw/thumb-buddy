//
// Created by Robert Harrison on 9/24/26.
//

#include <thumbbuddy/Logging.h>

extern "C" {
#include <libavutil/log.h>
}

namespace {
    int toAvLogLevel(const thumbbuddy::LogLevel level) {
        switch (level) {
            case thumbbuddy::LogLevel::quiet:   return AV_LOG_QUIET;
            case thumbbuddy::LogLevel::error:   return AV_LOG_ERROR;
            case thumbbuddy::LogLevel::warning: return AV_LOG_WARNING;
            case thumbbuddy::LogLevel::info:    return AV_LOG_INFO;
            case thumbbuddy::LogLevel::debug:   return AV_LOG_DEBUG;
        }
        return AV_LOG_ERROR;
    }
}

namespace thumbbuddy {
    void setLogLevel(const LogLevel level) {
        av_log_set_level(toAvLogLevel(level));
    }
}
