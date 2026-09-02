//
// Created by Robert Harrison on 9/1/26.
//

#ifndef THUMBBUDDY_FRAMERATE_H
#define THUMBBUDDY_FRAMERATE_H

#include <string>
#include <format>

namespace thumbbuddy {
    struct FrameRate {
        int numerator {};
        int denominator {1};

        [[nodiscard]]
        constexpr double fps() const {
            return static_cast<double>(numerator) / denominator;
        }

        [[nodiscard]]
        std::string toRationalString() const {
            return std::to_string(numerator) + "/" + std::to_string(denominator);
        }

        [[nodiscard]]
        std::string toString() const {
            return std::format("{:.5g}", fps());
        }

        friend bool operator==(const FrameRate& a, const FrameRate& b) = default;
    };

    // Required for Google Test to output readable errors instead of hex dumps.
    inline std::ostream& operator<<(std::ostream& os, const FrameRate& frameRate) {
        return os << frameRate.toRationalString();
    }
}

#endif //THUMBBUDDY_FRAMERATE_H
