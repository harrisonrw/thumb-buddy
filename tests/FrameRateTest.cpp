//
// Created by Robert Harrison on 9/1/26.
//

#include <thumbbuddy/FrameRate.h>
#include <gtest/gtest.h>

TEST(FrameRate, DefaultsToZeroFps) {
    const thumbbuddy::FrameRate frameRate {};
    EXPECT_EQ(frameRate.numerator, 0);
    EXPECT_EQ(frameRate.denominator, 1);
}

TEST(FrameRate, PartialInitializationDefaultsDenominatorToOne) {
    constexpr thumbbuddy::FrameRate frameRate { 30 };
    EXPECT_EQ(frameRate.denominator, 1);
    EXPECT_DOUBLE_EQ(frameRate.fps(), 30.0);
}

TEST(FrameRate, NtscFps) {
    constexpr thumbbuddy::FrameRate ntsc { 30000, 1001 };
    EXPECT_NEAR(ntsc.fps(), 29.97, 0.001);
}

TEST(FrameRate, ToRationalString) {
    constexpr thumbbuddy::FrameRate frameRate { 30000, 1001 };
    EXPECT_EQ(frameRate.toRationalString(), "30000/1001");
}

TEST(FrameRate, ToString) {
    EXPECT_EQ((thumbbuddy::FrameRate { 30000, 1001 }).toString(), "29.97");
    EXPECT_EQ((thumbbuddy::FrameRate { 24000, 1001 }).toString(), "23.976");
    EXPECT_EQ((thumbbuddy::FrameRate { 60000, 1001 }).toString(), "59.94");
    EXPECT_EQ((thumbbuddy::FrameRate { 30, 1 }).toString(), "30");
}

TEST(FrameRate, ComparesMemberwise) {
    EXPECT_EQ((thumbbuddy::FrameRate { 30000, 1001 }), (thumbbuddy::FrameRate { 30000, 1001 }));
    EXPECT_NE((thumbbuddy::FrameRate { 30000, 1001 }), (thumbbuddy::FrameRate { 60000, 2002 }));
}