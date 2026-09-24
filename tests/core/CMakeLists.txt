add_executable(
        thumbbuddy_core_tests
        FrameCandidateTest.cpp
        FrameRateTest.cpp
        MediaInfoTest.cpp
        MediaReaderTest.cpp
)

target_link_libraries(
        thumbbuddy_core_tests
        PRIVATE
        ThumbBuddy::core
        GTest::gtest_main
)

gtest_discover_tests(thumbbuddy_core_tests PROPERTIES LABELS core)
