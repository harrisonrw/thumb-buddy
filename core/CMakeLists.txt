add_library(thumbbuddy_core)
add_library(ThumbBuddy::core ALIAS thumbbuddy_core)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/thumbbuddy/Version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/thumbbuddy/Version.h
        @ONLY
)

find_package(PkgConfig REQUIRED)

# Floor is FFmpeg 8.0, which apt provides on Ubuntu 26.04 -- the release CI
# pins for exactly this reason. Raising it further means CI can no longer
# install FFmpeg from a package manager.
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET GLOBAL
        libavformat>=62  # FFmpeg 8.0
        libavcodec>=62   # FFmpeg 8.0
        libavutil>=60    # FFmpeg 8.0
        libswscale>=9    # FFmpeg 8.0
)

target_sources(
        thumbbuddy_core
        PUBLIC
        FILE_SET generated_headers TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/generated
        FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/thumbbuddy/Version.h
)

target_sources(
        thumbbuddy_core
        PRIVATE
        src/FrameAnalyzer.cpp
        src/ThumbnailGenerator.cpp
        src/MediaReader.cpp
        src/ColorDepth.cpp
        src/Logging.cpp
)

target_sources(
        thumbbuddy_core
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
        include/thumbbuddy/FrameRate.h
        include/thumbbuddy/ImageBuffer.h
        include/thumbbuddy/MediaInfo.h
        include/thumbbuddy/MediaType.h
        include/thumbbuddy/FrameCandidate.h
        include/thumbbuddy/FrameAnalyzer.h
        include/thumbbuddy/ThumbnailGenerator.h
        include/thumbbuddy/MediaReader.h
        include/thumbbuddy/ColorDepth.h
        include/thumbbuddy/Logging.h
)

target_compile_features(
        thumbbuddy_core
        PUBLIC
        cxx_std_20
)

target_link_libraries(
        thumbbuddy_core
        PRIVATE
        thumbbuddy_warnings
        PkgConfig::FFMPEG
)

install(
        TARGETS thumbbuddy_core
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET generated_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
