#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>
#include <random>
#include <string>

namespace junox {

/**
 * Low Frequency Oscillator.
 */
class LFO {
public:
    LFO(float sampleRate)
        : oneOverSampleRate(1.0f / sampleRate)
        , phaseIncrement(0.0f)
        , currentPhase(1.0f)
        , currentValue(0.0f)
        , isRestarted(false)
        , waveform(Waveform::SINE)
        , gen(std::random_device{}())
        , dist(-1.0f, 1.0f)
    {
    }

    enum class Waveform {
        NONE,
        TRIANGLE,
        SQUARE,
        SINE,
        RANDOM,
        NOISE
    };

    void reset() {
        currentPhase = 1.0f;
        currentValue = 0.0f;
    }

    float render() {
        // Increment the phase
        isRestarted = false;
        currentPhase += phaseIncrement;
        if (currentPhase > 1.0f) {
            isRestarted = true;
            currentPhase -= 1.0f;
        }

        // Convert phase to output waveform
        float value = 0.0f;
        switch (waveform) {
            case Waveform::NONE:
                value = 0.0f;
                break;
            case Waveform::SINE:
                value = std::sin(currentPhase * 2.0f * M_PI);
                break;
            case Waveform::SQUARE:
                value = currentPhase > 0.5f ? -1.0f : 1.0f;
                break;
            case Waveform::RANDOM:
                value = isRestarted ? dist(gen) : currentValue;
                break;
            case Waveform::NOISE:
                value = dist(gen);
                break;
            default: // Triangle
                value = currentPhase * 4.0f;
                if (value > 1.0f) value = 2.0f - value;
                if (value < -1.0f) value = -2.0f - value;
                break;
        }

        currentValue = value;
        return value;
    }

    void setRate(float frequency) {
        phaseIncrement = frequency * oneOverSampleRate;
    }

    void setWaveform(Waveform wf) {
        waveform = wf;
    }

    float getCurrentPhase() const { return currentPhase; }
    bool getIsRestarted() const { return isRestarted; }

private:
    float oneOverSampleRate;
    float phaseIncrement;
    float currentPhase;
    float currentValue;
    bool isRestarted;
    Waveform waveform;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;
};

} // namespace junox
