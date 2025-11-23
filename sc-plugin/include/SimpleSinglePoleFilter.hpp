#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>

namespace junox {

/**
 * Simple single-pole low-pass filter.
 */
class SimpleSinglePoleFilter {
public:
    SimpleSinglePoleFilter(float sampleRate, float fc = 1000.0f)
        : sampleRate(sampleRate)
        , z1(0.0f)
    {
        setCutoff(fc);
    }

    void setCutoff(float fc) {
        b1 = -std::exp((-2.0f * fc * M_PI) / sampleRate);
        a0 = 1.0f + b1;
    }

    float renderLP(float input) {
        const float xout = input * a0 - z1;
        z1 = b1 * xout;
        return xout;
    }

    void reset() {
        z1 = 0.0f;
    }

private:
    float sampleRate;
    float b1;
    float a0;
    float z1;
};

} // namespace junox
