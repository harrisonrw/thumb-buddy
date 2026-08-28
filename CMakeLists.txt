cmake_minimum_required(VERSION 4.2)

project(
        ThumbBuddy
        VERSION 0.1
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(thumbbuddy_warnings INTERFACE)
target_compile_options(thumbbuddy_warnings INTERFACE -Wall -Wextra)

add_subdirectory(core)
add_subdirectory(cli)

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
