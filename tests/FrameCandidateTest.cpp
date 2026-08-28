//
// Created by Robert Harrison on 8/28/26.
//

#include <thumbbuddy/FrameCandidate.h>
#include <gtest/gtest.h>

TEST(FrameCandidate, DefaultsToZero) {
    const thumbbuddy::FrameCandidate candidate {};
    EXPECT_DOUBLE_EQ(candidate.timestamp, 0.0);
}

TEST(FrameCandidate, AggregateInitialisationOrder) {
    const thumbbuddy::FrameCandidate candidate { 1.0, 2.0, 3.0, 4.0 };
    EXPECT_DOUBLE_EQ(candidate.timestamp, 1.0);
    EXPECT_DOUBLE_EQ(candidate.sharpness, 2.0);
    EXPECT_DOUBLE_EQ(candidate.brightness, 3.0);
    EXPECT_DOUBLE_EQ(candidate.motion, 4.0);
}