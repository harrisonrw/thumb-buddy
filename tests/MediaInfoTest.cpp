//
// Created by Robert Harrison on 9/2/26.
//

#include <thumbbuddy/MediaInfo.h>
#include <gtest/gtest.h>

TEST(MediaInfo, DefaultsToUnknown) {
    const thumbbuddy::MediaInfo mediaInfo {};
    EXPECT_EQ(mediaInfo.mediaType, thumbbuddy::MediaType::unknown);
    EXPECT_EQ(mediaInfo.width, 0);
    EXPECT_EQ(mediaInfo.height, 0);
    EXPECT_EQ(mediaInfo.duration, std::nullopt);
    EXPECT_EQ(mediaInfo.frameRate, std::nullopt);
    EXPECT_EQ(mediaInfo.codec, std::nullopt);
    EXPECT_EQ(mediaInfo.bitRate, std::nullopt);
}

TEST(MediaInfo, InitializationWithImage) {
    const thumbbuddy::MediaInfo mediaInfo { thumbbuddy::MediaType::image, 1024, 768 };
    EXPECT_EQ(mediaInfo.mediaType, thumbbuddy::MediaType::image);
    EXPECT_EQ(mediaInfo.width, 1024);
    EXPECT_EQ(mediaInfo.height, 768);
    EXPECT_EQ(mediaInfo.duration, std::nullopt);
    EXPECT_EQ(mediaInfo.frameRate, std::nullopt);
    EXPECT_EQ(mediaInfo.codec, std::nullopt);
    EXPECT_EQ(mediaInfo.bitRate, std::nullopt);
}

TEST(MediaInfo, InitializationWithVideo) {
    const thumbbuddy::FrameRate frameRate { 30000, 1001 };

    const thumbbuddy::MediaInfo mediaInfo {
        thumbbuddy::MediaType::video,
        1024,
        768,
        30.0,
        frameRate,
        "h264",
        60000
    };

    EXPECT_EQ(mediaInfo.mediaType, thumbbuddy::MediaType::video);
    EXPECT_EQ(mediaInfo.width, 1024);
    EXPECT_EQ(mediaInfo.height, 768);
    EXPECT_DOUBLE_EQ(mediaInfo.duration.value(), 30.0);
    EXPECT_EQ(mediaInfo.frameRate.value(), frameRate);
    EXPECT_EQ(mediaInfo.codec, "h264");
    EXPECT_EQ(mediaInfo.bitRate.value(), 60000);
}

TEST(MediaInfo, VideoWithUnknownFrameRate) {
    const thumbbuddy::MediaInfo mediaInfo {
        thumbbuddy::MediaType::video,
        1024,
        768,
        30.0,
        std::nullopt,
        "h264",
        60000
    };

    EXPECT_EQ(mediaInfo.mediaType, thumbbuddy::MediaType::video);
    EXPECT_DOUBLE_EQ(mediaInfo.duration.value(), 30.0);
    EXPECT_EQ(mediaInfo.frameRate, std::nullopt);
    EXPECT_EQ(mediaInfo.codec, "h264");
    EXPECT_EQ(mediaInfo.bitRate.value(), 60000);
}