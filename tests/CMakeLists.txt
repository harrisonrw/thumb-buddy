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

include(GoogleTest)

# One test binary per layer, so that core's tests cannot quietly grow a
# dependency on the CLI. Run a single layer with `ctest -L core` / `-L cli`.
add_subdirectory(core)
add_subdirectory(cli)
