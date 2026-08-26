//
// Created by Robert Harrison on 8/23/26.
//

#include <iostream>
#include <string_view>
#include <thumbbuddy/FrameCandidate.h>
#include <thumbbuddy/Version.h>

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        const std::string_view arg = argv[i];
        if (arg == "--version") {
            std::cout << "thumbbuddy version " << thumbbuddy::kVersion << "\n";
            return 0;
        }
    }

    std::cout << "Thumb Buddy\n";

    thumbbuddy::FrameCandidate frameCandidate = {  0.0, 0.20, 0.50, 0.10 };

    return 0;
}