add_executable(
        thumbbuddy_cli_tests
        FormatTest.cpp
)

target_link_libraries(
        thumbbuddy_cli_tests
        PRIVATE
        thumbbuddy_cli
        GTest::gtest_main
)

gtest_discover_tests(thumbbuddy_cli_tests PROPERTIES LABELS cli)
