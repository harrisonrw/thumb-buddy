//
// Created by Robert Harrison on 10/6/26.
//

#include <thumbbuddy/ImageBuffer.h>
#include <gtest/gtest.h>
#include <utility>

namespace {
    constexpr std::uint32_t kBytesPerPixel = 4;

    thumbbuddy::ImageBuffer makeTestImageBuffer(const std::uint32_t width, const std::uint32_t height) {
        const std::uint32_t stride = width * kBytesPerPixel;
        std::vector<std::uint8_t> pixels(stride * height);

        for (std::uint32_t y = 0; y < height; y++) {
            for (std::uint32_t x = 0; x < width; x++) {
                const std::size_t offset = y * stride + x * kBytesPerPixel;
                pixels[offset] = static_cast<std::uint8_t>(x);
                pixels[offset + 1] = static_cast<std::uint8_t>(y);
                pixels[offset + 2] = static_cast<std::uint8_t>(x + y);
                pixels[offset + 3] = 255;
            }
        }

        return { width, height, stride, thumbbuddy::PixelFormat::RGBA8, std::move(pixels) };
    }
}

TEST(ImageBuffer, Initialization) {
    const thumbbuddy::ImageBuffer imageBuffer = makeTestImageBuffer(4, 3);
    EXPECT_EQ(imageBuffer.width, 4u);
    EXPECT_EQ(imageBuffer.height, 3u);
    EXPECT_EQ(imageBuffer.stride, 16u);
    EXPECT_EQ(imageBuffer.pixelFormat, thumbbuddy::PixelFormat::RGBA8);
    EXPECT_EQ(imageBuffer.pixels.size(), 48u);

    // Spot check pixel at (x=2, y=1).
    const std::size_t offset = 1 * imageBuffer.stride + 2 * kBytesPerPixel;
    EXPECT_EQ(imageBuffer.pixels[offset], 2u);
    EXPECT_EQ(imageBuffer.pixels[offset + 1], 1u);
    EXPECT_EQ(imageBuffer.pixels[offset + 2], 3u);
    EXPECT_EQ(imageBuffer.pixels[offset + 3], 255u);
}