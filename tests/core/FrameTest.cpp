//
// Created by Robert Harrison on 10/12/26.
//

#include <thumbbuddy/Frame.h>
#include <gtest/gtest.h>
#include <utility>

TEST(Frame, Initialization) {
    const thumbbuddy::Frame frame {};
    EXPECT_DOUBLE_EQ(frame.timestamp, 0.0);
    EXPECT_EQ(frame.image.width, 0u);
    EXPECT_EQ(frame.image.height, 0u);
    EXPECT_TRUE(frame.image.pixels.empty());
}
