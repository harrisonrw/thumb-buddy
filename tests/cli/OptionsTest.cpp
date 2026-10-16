//
// Created by Robert Harrison on 10/16/26.
//

#include <Options.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

using thumbbuddy::cli::parseArgs;

namespace {
    // parseArgs takes argv-style pointers, and Options::inputs views into them, so the
    // strings must outlive the parsed Options.
    struct Args {
        std::vector<std::string> storage;
        std::vector<char *> pointers;

        Args(std::initializer_list<const char *> args) : storage(args.begin(), args.end()) {
            for (std::string &arg : storage) {
                pointers.push_back(arg.data());
            }
        }

        [[nodiscard]] std::span<char * const> span() const { return pointers; }
    };
}

TEST(ParseArgsOutput, ShortFlagBeforeInput) {
    const Args args { "-o", "out", "a.mp4" };
    const thumbbuddy::cli::Options options = parseArgs(args.span());

    EXPECT_FALSE(options.error.has_value());
    EXPECT_EQ(options.outputDirectory, "out");
    ASSERT_EQ(options.inputs.size(), 1u);
    EXPECT_EQ(options.inputs[0], "a.mp4");
}

TEST(ParseArgsOutput, LongFlagAfterInput) {
    const Args args { "a.mp4", "--output", "out" };
    const thumbbuddy::cli::Options options = parseArgs(args.span());

    EXPECT_FALSE(options.error.has_value());
    EXPECT_EQ(options.outputDirectory, "out");
    ASSERT_EQ(options.inputs.size(), 1u);
    EXPECT_EQ(options.inputs[0], "a.mp4");
}

TEST(ParseArgsOutput, MissingValueIsAnError) {
    const Args args { "a.mp4", "-o" };
    const thumbbuddy::cli::Options options = parseArgs(args.span());

    EXPECT_TRUE(options.error.has_value());
}

TEST(ParseArgsOutput, OmittedLeavesOutputDirectoryEmpty) {
    const Args args { "a.mp4" };
    const thumbbuddy::cli::Options options = parseArgs(args.span());

    EXPECT_FALSE(options.error.has_value());
    EXPECT_FALSE(options.outputDirectory.has_value());
}
