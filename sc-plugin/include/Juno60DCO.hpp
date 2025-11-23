#pragma once

#include <cmath>

namespace junox {

/**
 * Juno-60 Digital Controlled Oscillator with PolyBLEP anti-aliasing.
 */
class Juno60DCO {
public:
    Juno60DCO(float sampleRate)
        : sampleRate(sampleRate)
        , currentPhase(0.0f)
        , phaseIncrement(0.0f)
        , pulseWidth(0.5f)
        , pulsePositive(1.0f)
        , pulseNegative(-1.0f)
        , pulseHeight(1.0f)
        , subOutput(1.0f)
    {
    }

    void noteOn(int noteNumber) {
        // Convert MIDI note number to frequency (A440 is 442Hz in Juno-60)
        const float noteFrequency = std::pow(2.0f, (noteNumber - 69) / 12.0f) * 442.0f;
        phaseIncrement = noteFrequency / sampleRate;
        
        // Juno-60 DCO starts partway through cycle
        currentPhase = 1.1f;
    }

    float render(float detuneFactor, float pulseWidthMod, 
                 float sawLevel, float pulseLevel, float subLevel) {
        // Increment phase with detuning
        const float phaseInc = phaseIncrement * detuneFactor;
        const float origPhase = currentPhase;
        currentPhase += phaseInc;
        
        if (currentPhase > 1.0f) {
            currentPhase -= 1.0f;
            
            // Update PWM only at phase wrap to avoid noise
            pulseWidth = 0.5f - 0.45f * pulseWidthMod;
            pulsePositive = 1.0f - pulseWidthMod * 0.95f;
            pulseNegative = -1.0f;
            pulseHeight = 0.45f * (pulsePositive - pulseNegative);
        }

        // Sawtooth with PolyBLEP
        float newSawOutput = 0.0f;
        if (sawLevel > 0.0f) {
            newSawOutput = currentPhase + currentPhase - 1.0f;
            newSawOutput -= calcPolyBLEP2(currentPhase, phaseInc, 1.0f);
        }

        // Pulse with PolyBLEP
        float newPulseOutput = 0.0f;
        if (pulseLevel > 0.0f) {
            newPulseOutput = currentPhase > pulseWidth ? (pulsePositive *= 0.998f) : (pulseNegative *= 0.998f);
            newPulseOutput -= calcPolyBLEP2(currentPhase, phaseInc, pulseHeight);
            const float x = currentPhase - pulseWidth;
            newPulseOutput += calcPolyBLEP2(x < 0.0f ? x + 1.0f : x, phaseInc, pulseHeight);
        }

        // Sub oscillator (flip-flops at phase 0.5)
        float newSubOutput = (subOutput *= 0.998f);
        float y = currentPhase - 0.5f;
        if (y < phaseInc && y > -phaseInc) {
            if (y < 0.0f) y += 1.0f;
            const float origSubOutput = newSubOutput;
            if (currentPhase >= 0.5f && origPhase < 0.5f) {
                subOutput = newSubOutput = (newSubOutput > 0.0f) ? -1.0f : 1.0f;
            }
            newSubOutput -= calcPolyBLEP2(y, phaseInc, origSubOutput);
        }

        return newSawOutput * sawLevel + newPulseOutput * pulseLevel + newSubOutput * subLevel;
    }

private:
    float calcPolyBLEP2(float phase, float inc, float height) {
        float result = 0.0f;
        if (phase < inc) {
            // Right side of transition
            const float t = phase / inc;
            result = height * (t + t - t * t - 1.0f);
        } else if (phase + inc > 1.0f) {
            // Left side of transition
            const float t = (phase - 1.0f) / inc;
            result = height * (t * t + (t + t) + 1.0f);
        }
        return result;
    }

    float sampleRate;
    float currentPhase;
    float phaseIncrement;
    float pulseWidth;
    float pulsePositive;
    float pulseNegative;
    float pulseHeight;
    float subOutput;
};

} // namespace junox
