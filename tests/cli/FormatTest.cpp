//
// Created by Robert Harrison on 9/24/26.
//

#include <Format.h>
#include <gtest/gtest.h>

using thumbbuddy::cli::formatDuration;
using thumbbuddy::cli::toMbps;

TEST(FormatDuration, Zero) {
    EXPECT_EQ(formatDuration(0.0), "0:00");
}

TEST(FormatDuration, PadsSecondsUnderOneMinute) {
    EXPECT_EQ(formatDuration(5.0), "0:05");
    EXPECT_EQ(formatDuration(59.0), "0:59");
}

TEST(FormatDuration, OmitsHoursUnderOneHour) {
    EXPECT_EQ(formatDuration(90.0), "1:30");
    EXPECT_EQ(formatDuration(3599.0), "59:59");
}

TEST(FormatDuration, IncludesHoursAndPadsMinutes) {
    EXPECT_EQ(formatDuration(3600.0), "1:00:00");
    EXPECT_EQ(formatDuration(3725.0), "1:02:05");
}

TEST(FormatDuration, DoesNotWrapHoursAtOneDay) {
    EXPECT_EQ(formatDuration(90000.0), "25:00:00");
}

TEST(FormatDuration, RoundsToNearestSecond) {
    EXPECT_EQ(formatDuration(59.4), "0:59");
    EXPECT_EQ(formatDuration(59.6), "1:00");
    EXPECT_EQ(formatDuration(3599.6), "1:00:00");  // rounding carries into the hours branch
}

TEST(ToMbps, UsesDecimalMegabits) {
    EXPECT_EQ(toMbps(2'500'000), "2.5 Mbps");
    EXPECT_EQ(toMbps(12'345'678), "12.3 Mbps");
}

TEST(ToMbps, FormatsWithOneDecimalAndUnit) {
    EXPECT_EQ(toMbps(0), "0.0 Mbps");
    EXPECT_EQ(toMbps(8'000'000), "8.0 Mbps");
    EXPECT_EQ(toMbps(100'000'000), "100.0 Mbps");
}

TEST(ToMbps, RoundsToNearestTenth) {
    EXPECT_EQ(toMbps(8'350'000), "8.3 Mbps");
    EXPECT_EQ(toMbps(8'450'000), "8.4 Mbps");
}