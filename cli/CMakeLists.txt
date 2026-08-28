add_executable(
        thumbbuddy
        src/main.cpp
        src/Options.h
        src/Options.cpp
)

target_link_libraries(
        thumbbuddy
        PRIVATE
        ThumbBuddy::core
)