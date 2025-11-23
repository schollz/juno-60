#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <random>

namespace junox {

/**
 * Pink noise generator with low-pass filter.
 */
class Noise {
public:
    Noise(float sampleRate, float fc = 5000.0f)
        : gen(std::random_device{}())
        , dist(-1.0f, 1.0f)
        , z1(0.0f)
    {
        // Coefficients for 6db low pass output filter
        b1 = -std::exp((-2.0f * fc * M_PI) / sampleRate);
        a0 = 1.0f + b1;
    }

    float render() {
        // White noise
        const float xin = dist(gen);
        
        // Apply low pass filter to convert to pink noise
        const float xout = xin * a0 - z1;
        z1 = b1 * xout;
        return xout;
    }

private:
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;
    float b1;
    float a0;
    float z1;
};

} // namespace junox
