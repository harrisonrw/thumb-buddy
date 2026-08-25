add_executable(
        thumbbuddy
        src/main.cpp
)

target_link_libraries(
        thumbbuddy
        PRIVATE
        thumbbuddy_core
)