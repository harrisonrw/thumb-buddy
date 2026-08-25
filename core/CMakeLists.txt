add_library(thumbbuddy_core)
add_library(ThumbBuddy::core ALIAS thumbbuddy_core)

target_sources(
        thumbbuddy_core
        PRIVATE
        src/FrameAnalyzer.cpp
        src/ThumbnailGenerator.cpp
)

target_sources(
        thumbbuddy_core
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
        include/thumbbuddy/FrameCandidate.h
        include/thumbbuddy/FrameAnalyzer.h
        include/thumbbuddy/ThumbnailGenerator.h
)

target_compile_features(
        thumbbuddy_core
        PUBLIC
        cxx_std_20
)
