//
// Created by Robert Harrison on 9/24/26.
//

#include <thumbbuddy/Logging.h>
#include <gtest/gtest.h>

extern "C" {
#include <libavutil/log.h>
}

namespace {
    // FFmpeg's log level is process-wide, so restore the original for the other tests.
    class Logging : public testing::Test {
    protected:
        void SetUp() override { original_ = av_log_get_level(); }
        void TearDown() override { av_log_set_level(original_); }

    private:
        int original_ {};
    };
}

TEST_F(Logging, Quiet) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::quiet);
    EXPECT_EQ(av_log_get_level(), AV_LOG_QUIET);
}

TEST_F(Logging, Error) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::error);
    EXPECT_EQ(av_log_get_level(), AV_LOG_ERROR);
}

TEST_F(Logging, Warning) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::warning);
    EXPECT_EQ(av_log_get_level(), AV_LOG_WARNING);
}

TEST_F(Logging, Info) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::info);
    EXPECT_EQ(av_log_get_level(), AV_LOG_INFO);
}

TEST_F(Logging, Debug) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::debug);
    EXPECT_EQ(av_log_get_level(), AV_LOG_DEBUG);
}

TEST_F(Logging, LevelsAreOrderedFromQuietToVerbose) {
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::quiet);
    const int quiet = av_log_get_level();
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::error);
    const int error = av_log_get_level();
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::warning);
    const int warning = av_log_get_level();
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::info);
    const int info = av_log_get_level();
    thumbbuddy::setLogLevel(thumbbuddy::LogLevel::debug);
    const int debug = av_log_get_level();

    EXPECT_LT(quiet, error);
    EXPECT_LT(error, warning);
    EXPECT_LT(warning, info);
    EXPECT_LT(info, debug);
}
