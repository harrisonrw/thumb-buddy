include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.0
)
FetchContent_MakeAvailable(googletest)

add_executable(thumbbuddy_tests FrameCandidateTest.cpp)
target_link_libraries(thumbbuddy_tests PRIVATE ThumbBuddy::core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(thumbbuddy_tests)
