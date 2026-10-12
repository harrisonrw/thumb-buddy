//
// Created by Robert Harrison on 10/12/26.
//

#include <thumbbuddy/FrameExtractor.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

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
