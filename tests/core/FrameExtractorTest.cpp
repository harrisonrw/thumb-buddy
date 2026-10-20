//
// Created by Robert Harrison on 10/12/26.
//

#include <thumbbuddy/FrameExtractor.h>
#include <thumbbuddy/Frame.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>
#include <cstdint>
#include <span>
#include <string>

namespace {
    // FNV-1a (64-bit) hashing function to detect any changed bytes.
    std::uint64_t fnv1a(std::span<const std::uint8_t> bytes, std::uint64_t hash = 0xcbf29ce484222325) {
        for (std::uint8_t byte : bytes) {
            hash = (hash ^ byte) * 0x100000001b3;
        }
        return hash;
    }

    std::uint64_t hashFrames(const std::vector<thumbbuddy::Frame> &frames) {
        std::uint64_t hash = fnv1a({});
        for (const thumbbuddy::Frame &frame : frames) {
            hash = fnv1a(frame.image.pixels, hash);
        }
        return hash;
    }

    std::optional<std::vector<thumbbuddy::Frame>> extractFixtureFrames(const char *name, double interval, std::string error) {
        const std::filesystem::path path = std::filesystem::path { THUMBBUDDY_FIXTURES_DIR } / name;

        std::optional<thumbbuddy::FrameExtractor> extractor = thumbbuddy::FrameExtractor::open(path, &error);
        if (!extractor) {
            return std::nullopt;
        }

        return extractor.value().extractFrames(interval, &error);
    }
}

TEST(FrameExtractor, NonExistentPathReturnsNoExtractor) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "thumbbuddy-no-such-file.mp4";
    std::string error;
    const std::optional<thumbbuddy::FrameExtractor> extractor = thumbbuddy::FrameExtractor::open(path, &error);

    EXPECT_FALSE(extractor.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(FrameExtractor, EmptyFileReturnsNoExtractor) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "thumbbuddy-empty-file.mp4";
    { const std::ofstream file { path }; }

    std::string error;
    const std::optional<thumbbuddy::FrameExtractor> extractor = thumbbuddy::FrameExtractor::open(path, &error);
    std::filesystem::remove(path);

    EXPECT_FALSE(extractor.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(FrameExtractor, Fixture8BitPixelHash) {
    std::string error;
    std::optional<std::vector<thumbbuddy::Frame>> frames = extractFixtureFrames("fixture-8bit.mp4", 0.5, error);
    ASSERT_TRUE(frames.has_value()) << error;
    ASSERT_EQ(frames.value().size(), 2u);

    const thumbbuddy::Frame &frameZero = (*frames)[0];
    EXPECT_DOUBLE_EQ(frameZero.timestamp, 0.0);
    EXPECT_EQ(frameZero.image.width, 320u);
    EXPECT_EQ(frameZero.image.height, 240u);

    EXPECT_EQ(hashFrames(*frames), 16241136530502737322ull) << "fixture-8bit.mp4 pixels changed: swscale output differs on this platform";
}

TEST(FrameExtractor, Fixture10BitPixelHash) {
    std::string error;
    std::optional<std::vector<thumbbuddy::Frame>> frames = extractFixtureFrames("fixture-10bit.mp4", 0.5, error);
    ASSERT_TRUE(frames.has_value()) << error;
    ASSERT_EQ(frames.value().size(), 2u);

    const thumbbuddy::Frame &frameZero = (*frames)[0];
    EXPECT_DOUBLE_EQ(frameZero.timestamp, 0.0);
    EXPECT_EQ(frameZero.image.width, 320u);
    EXPECT_EQ(frameZero.image.height, 240u);

    EXPECT_EQ(hashFrames(*frames), 8868152357443850592ull) << "fixture-10bit.mp4 pixels changed: swscale output differs on this platform";
}

TEST(FrameExtractor, Fixture8BitTimestamps) {
    std::string error;
    std::optional<std::vector<thumbbuddy::Frame>> frames = extractFixtureFrames("fixture-8bit.mp4", 0.5, error);
    ASSERT_TRUE(frames.has_value()) << error;
    ASSERT_EQ(frames.value().size(), 2u);

    EXPECT_DOUBLE_EQ((*frames)[0].timestamp, 0.0);
    EXPECT_DOUBLE_EQ((*frames)[1].timestamp, 0.48);
}
