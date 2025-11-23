#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>

namespace junox {

/**
 * Parameter smoothing to avoid zipper noise when changing values.
 */
class SmoothMoves {
public:
    SmoothMoves(float value, float sampleRate, float fc = 5.0f)
        : targetValue(value)
        , isStarted(false)
        , z1(0.0f)
    {
        b1 = -std::exp((-2.0f * fc * M_PI) / sampleRate);
        a0 = 1.0f + b1;
        reset();
    }

    void setValue(float value, bool useSmoothing = true) {
        targetValue = value;
        if (!isStarted || !useSmoothing) {
            reset();
        }
    }

    void reset() {
        z1 = targetValue * a0 - targetValue;
        isStarted = false;
    }

    float getNextValue() {
        isStarted = true;
        const float xout = targetValue * a0 - z1;
        z1 = b1 * xout;
        return xout;
    }

private:
    float b1;
    float a0;
    float targetValue;
    bool isStarted;
    float z1;
};

} // namespace junox
