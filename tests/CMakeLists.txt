include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.0
)
# Keep GoogleTest's own install rules out of our install tree / packages.
set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)
FetchContent_MakeAvailable(googletest)

add_executable(
        thumbbuddy_tests
        FrameCandidateTest.cpp
        FrameRateTest.cpp
        MediaInfoTest.cpp
)

target_link_libraries(thumbbuddy_tests PRIVATE ThumbBuddy::core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(thumbbuddy_tests)
