add_library(thumbbuddy_core)

target_sources(
        thumbbuddy_core
        PRIVATE
        src/FrameAnalyzer.cpp
        src/ThumbnailGenerator.cpp
)

target_include_directories(
        thumbbuddy_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
        thumbbuddy_core
        PUBLIC
        cxx_std_20
)