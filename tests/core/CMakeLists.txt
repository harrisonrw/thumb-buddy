add_executable(
        thumbbuddy_core_tests
        ColorDepthTest.cpp
        FrameCandidateTest.cpp
        FrameExtractorTest.cpp
        FrameRateTest.cpp
        FrameTest.cpp
        LoggingTest.cpp
        ImageBufferTest.cpp
        MediaInfoTest.cpp
        MediaReaderTest.cpp
)

target_link_libraries(
        thumbbuddy_core_tests
        PRIVATE
        ThumbBuddy::core
        GTest::gtest_main
        PkgConfig::FFMPEG
)

target_compile_definitions(thumbbuddy_core_tests PRIVATE
        THUMBBUDDY_FIXTURES_DIR="${PROJECT_SOURCE_DIR}/tests/fixtures")

gtest_discover_tests(thumbbuddy_core_tests PROPERTIES LABELS core)
