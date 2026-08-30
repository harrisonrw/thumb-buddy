add_library(thumbbuddy_core)
add_library(ThumbBuddy::core ALIAS thumbbuddy_core)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/thumbbuddy/Version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/thumbbuddy/Version.h
        @ONLY
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

target_link_libraries(thumbbuddy_core PRIVATE thumbbuddy_warnings)

install(
        TARGETS thumbbuddy_core
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET generated_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
