add_library(thumbbuddy_cli
        STATIC
        src/Options.cpp
        src/Format.cpp
)

target_include_directories(
        thumbbuddy_cli
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
        thumbbuddy_cli
        PUBLIC
        ThumbBuddy::core
        PRIVATE
        thumbbuddy_warnings
)

add_executable(
        thumbbuddy
        src/main.cpp
)

target_link_libraries(
        thumbbuddy
        PRIVATE
        thumbbuddy_cli
        thumbbuddy_warnings
)

install(
        TARGETS thumbbuddy
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
