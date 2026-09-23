//
// Created by Robert Harrison on 9/17/26.
//

#include <thumbbuddy/MediaReader.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

TEST(MediaReader, NonExistentPathReturnsNoReader) {
    const std::filesystem::path path = std::filesystem::temp_directory_path()/ "thumbbuddy-no-such-file.mp4";
    std::string error;
    const std::optional<thumbbuddy::MediaReader> reader = thumbbuddy::MediaReader::open(path, &error);

    EXPECT_FALSE(reader.has_value());
    EXPECT_FALSE(error.empty());
}
