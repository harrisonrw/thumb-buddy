cmake_minimum_required(VERSION 4.2)

project(
        ThumbBuddy
        VERSION 0.1
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

add_library(thumbbuddy_warnings INTERFACE)
target_compile_options(thumbbuddy_warnings INTERFACE -Wall -Wextra)

add_subdirectory(core)
add_subdirectory(cli)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Packaging. Guarded so that consuming projects embedding ThumbBuddy (e.g. via FetchContent) do not inherit our CPack configuration.
if (PROJECT_IS_TOP_LEVEL)
    set(CPACK_PACKAGE_NAME thumbbuddy)
    set(CPACK_PACKAGE_VENDOR "RWH Technology")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Video thumbnail generator")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_GENERATOR TGZ)
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_VERBATIM_VARIABLES TRUE)

    include(CPack)
endif()
