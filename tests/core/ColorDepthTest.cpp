//
// Created by Robert Harrison on 9/24/26.
//

#include <thumbbuddy/ColorDepth.h>
#include <gtest/gtest.h>

extern "C" {
#include <libavutil/pixfmt.h>
}

TEST(ColorDepth, EightBitVideoFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_YUV420P, 0), 8);
}

TEST(ColorDepth, TenBitVideoFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_YUV420P10LE, 0), 10);
}

TEST(ColorDepth, TwelveBitVideoFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_YUV420P12LE, 0), 12);
}

TEST(ColorDepth, SixteenBitImageFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_RGB48BE, 0), 16);
}

TEST(ColorDepth, PalettizedFormatReportsIndexDepth) {
    // pal8 stores an 8-bit index rather than 8-bit colour, but the palette
    // entries are themselves 8-bit per channel, so 8 is the useful answer.
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_PAL8, 0), 8);
}

TEST(ColorDepth, MonochromeFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_MONOBLACK, 0), 1);
}

TEST(ColorDepth, PackedFormatReportsBitsPerComponentNotPerPixel) {
    // Guards against reaching for av_get_bits_per_pixel, which answers 24 here.
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_RGB24, 0), 8);
}

TEST(ColorDepth, SubsampledFormatReportsBitsPerComponentNotPerPixel) {
    // av_get_bits_per_pixel answers 15 for this one.
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_YUV420P10LE, 0), 10);
}

TEST(ColorDepth, PixelFormatWinsOverRawSampleDepth) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_YUV420P10LE, 8), 10);
}

TEST(ColorDepth, FallsBackToRawSampleDepthWithoutPixelFormat) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_NONE, 10), 10);
}

TEST(ColorDepth, UnknownWithoutPixelFormatOrRawSampleDepth) {
    EXPECT_EQ(thumbbuddy::colorDepthFor(AV_PIX_FMT_NONE, 0), std::nullopt);
}
