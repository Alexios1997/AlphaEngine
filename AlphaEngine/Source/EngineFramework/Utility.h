#pragma once

#include <math.h>

namespace AlphaEngine::Math {
    template<typename T>
    // Try to calculate the result of this function/variable at compile time, not while the game is running
    constexpr T Lerp(T a, T b, float t) {
        return a + t * (b - a);
    }

    float EaseInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }
}