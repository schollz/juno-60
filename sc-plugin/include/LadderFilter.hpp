#pragma once

#include <cmath>

namespace junox {

/**
 * Moog-style Virtual Analog ladder filter.
 */
class LadderFilter {
public:
    LadderFilter(float sampleRate)
        : nyquistLimit(sampleRate * 0.5f)
        , piOverSampleRate(M_PI / sampleRate)
        , z1(0.0f)
        , z2(0.0f)
        , z3(0.0f)
        , z4(0.0f)
    {
    }

    void reset() {
        z1 = 0.0f;
        z2 = 0.0f;
        z3 = 0.0f;
        z4 = 0.0f;
    }

    float calcCutoffFactor(float fc) {
        if (fc > nyquistLimit) {
            fc = nyquistLimit;
        }
        return std::tan(fc * piOverSampleRate);
    }

    void trigger(float initialExcite) {
        z4 += initialExcite;
    }

    float process(float input, float cutoffFactor, float resonance) {
        const float oneOverOnePlusg = 1.0f / (1.0f + cutoffFactor);
        
        // Feedforward coefficient
        const float alpha = cutoffFactor * oneOverOnePlusg;
        
        // Feedback coefficients
        const float beta4 = oneOverOnePlusg;
        const float beta3 = beta4 * alpha;
        const float beta2 = beta3 * alpha;
        const float beta1 = beta2 * alpha;
        
        // Mix feedback with input
        const float feedback = beta1 * z1 + beta2 * z2 + beta3 * z3 + beta4 * z4;
        const float k = 4.0f * resonance;
        const float alpha4 = alpha * alpha * alpha * alpha;
        const float xin = (input - k * feedback) / (1.0f + k * alpha4);
        
        // Apply pole 1
        const float lpf1In = (xin - z1) * alpha;
        const float lpf1Out = lpf1In + z1;
        z1 = lpf1In + lpf1Out;
        
        // Apply pole 2
        const float lpf2In = (lpf1Out - z2) * alpha;
        const float lpf2Out = lpf2In + z2;
        z2 = lpf2In + lpf2Out;
        
        // Apply pole 3
        const float lpf3In = (lpf2Out - z3) * alpha;
        const float lpf3Out = lpf3In + z3;
        z3 = lpf3In + lpf3Out;
        
        // Apply pole 4
        const float lpf4In = (lpf3Out - z4) * alpha;
        const float lpf4Out = lpf4In + z4;
        z4 = lpf4In + lpf4Out;
        
        // Return LPF4 output
        return lpf4Out;
    }

private:
    float nyquistLimit;
    float piOverSampleRate;
    float z1, z2, z3, z4;
};

} // namespace junox
