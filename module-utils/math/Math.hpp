#pragma once

#include <cstdint>
#include <cmath>

namespace trigonometry
{
    using SideLength = std::int32_t;
    using Degrees    = std::int32_t;
    using Radians    = double;

    constexpr Degrees FullAngle  = 360;
    constexpr Degrees HalfAngle  = 180;
    constexpr Degrees RightAngle = 90;

    constexpr static inline auto toRadians(Degrees degrees) noexcept -> Radians
    {
        return degrees * M_PI / HalfAngle;
    }

    struct AdjacentSide
    {
        static auto fromAngle(Radians angle, SideLength hypotenuse) -> SideLength
        {
            return std::lround(std::cos(angle) * hypotenuse);
        }

        static auto fromCosine(double cosine, SideLength hypotenuse) -> SideLength
        {
            return std::lround(cosine * hypotenuse);
        }
    };

    struct OppositeSide
    {
        static auto fromAngle(Radians angle, SideLength hypotenuse) -> SideLength
        {
            return std::lround(std::sin(angle) * hypotenuse);
        }

        static auto fromSine(double sine, SideLength hypotenuse) -> SideLength
        {
            return std::lround(sine * hypotenuse);
        }
    };
} // namespace trigonometry
