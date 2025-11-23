#pragma once

#include "RingBuffer.hpp"
#include "SimpleSinglePoleFilter.hpp"
#include <cmath>
#include <algorithm>

namespace junox {

/**
 * Roland Juno-60 chorus effect emulation.
 */
class Chorus {
public:
    Chorus(float sampleRate)
        : sampleRate(sampleRate)
        , leftOutput(0.0f)
        , rightOutput(0.0f)
        , isUsed(false)
        , nextChorusMode(0)
        , ringBuffer(static_cast<int>(sampleRate * 0.006f))
        , preFilter(sampleRate, 7237.0f)
        , postLeftFilter(sampleRate, 10644.0f)
        , postRightFilter(sampleRate, 10644.0f)
        , dryCurrent(1.0f)
        , dryChange(0.0f)
        , dryTarget(1.0f)
        , lfoValue(0.0f)
        , lfoIncrement(0.01f)
        , maxLeftOffset(0.0f)
        , averageLeftSamples(0.0f)
        , maxRightOffset(0.0f)
        , averageRightSamples(0.0f)
    {
    }

    void render(float input) {
        isUsed = true;
        float dry = dryCurrent;

        // Handle transitions to wet/dry ratio
        if (dryChange != 0.0f) {
            dry += dryChange;
            if (dry > 1.0f) {
                dry = 1.0f;
                dryChange = 0.0f;
                update(nextChorusMode);
            } else if (dry < dryTarget && dryChange < 0.0f) {
                dry = dryTarget;
                dryChange = 0.0f;
            }
            dryCurrent = dry;
        }

        // Fully dry mode
        if (dry == 1.0f) {
            leftOutput = input;
            rightOutput = input;
            return;
        }

        // Update LFO
        float lfoVal = lfoValue + lfoIncrement;
        if (lfoVal > 1.0f || lfoVal < -1.0f) {
            lfoIncrement = -lfoIncrement;
            lfoVal = lfoValue + lfoIncrement;
        }
        lfoValue = lfoVal;

        // Calculate left/right outputs
        const float dryOutput = input * dry;
        const float wetFactor = 1.0f - dry;

        const float leftDelaySamples = averageLeftSamples + lfoValue * maxLeftOffset;
        const float leftDelayedValue = ringBuffer.readSample(leftDelaySamples);
        leftOutput = dryOutput + postLeftFilter.renderLP(leftDelayedValue * wetFactor);

        const float rightDelaySamples = averageRightSamples + lfoValue * maxRightOffset;
        const float rightDelayedValue = ringBuffer.readSample(rightDelaySamples);
        rightOutput = dryOutput + postRightFilter.renderLP(rightDelayedValue * wetFactor);

        // Add input to ring buffer
        ringBuffer.writeSample(preFilter.renderLP(applySaturation(input)));
    }

    void reset() {
        ringBuffer.reset();
        preFilter.reset();
        postLeftFilter.reset();
        postRightFilter.reset();
        isUsed = false;
    }

    void update(int chorusMode) {
        if (dryCurrent < 1.0f && !isUsed) {
            // Avoid clicks - transition to fully dry first
            dryChange = 0.0005f;
            dryTarget = 1.0f;
            nextChorusMode = chorusMode;
        } else {
            switch (chorusMode) {
                case 1: // Mode I
                    updateValues(0.513f, 0.44f, 0.00154f, 0.00515f, 0.00151f, 0.0054f, true);
                    break;
                case 2: // Mode II
                    updateValues(0.863f, 0.44f, 0.00154f, 0.00515f, 0.00151f, 0.0054f, true);
                    break;
                case 3: // Mode I+II
                    updateValues(9.75f, 0.44f, 0.00322f, 0.00356f, 0.00328f, 0.00365f, false);
                    break;
                default: // Off
                    updateValues(0.513f, 1.0f, 0.00154f, 0.00515f, 0.00151f, 0.0054f, true);
                    ringBuffer.reset();
                    break;
            }
        }
    }

    float getLeftOutput() const { return leftOutput; }
    float getRightOutput() const { return rightOutput; }

private:
    float applySaturation(float input) {
        return input; // Could add tanh saturation here
    }

    void updateValues(float freq, float dry, float minLeftDelay, float maxLeftDelay,
                      float minRightDelay, float maxRightDelay, bool isStereo) {
        // Left delay
        const float averageLeftDelay = (minLeftDelay + maxLeftDelay) * 0.5f;
        const float maxLeftOff = maxLeftDelay - averageLeftDelay;
        averageLeftSamples = averageLeftDelay * sampleRate;
        maxLeftOffset = maxLeftOff * sampleRate;

        // Right delay
        const float averageRightDelay = (minRightDelay + maxRightDelay) * 0.5f;
        const float maxRightOff = maxRightDelay - averageRightDelay;
        averageRightSamples = averageRightDelay * sampleRate;
        maxRightOffset = maxRightOff * sampleRate * (isStereo ? -1.0f : 1.0f);

        // Transition to desired wet/dry ratio
        dryTarget = dry;
        if (!isUsed) {
            dryCurrent = dry;
        }
        dryChange = (dry - dryCurrent) / 1000.0f;

        // LFO increment
        lfoIncrement = (lfoIncrement >= 0.0f ? 1.0f : -1.0f) * 4.0f * freq / sampleRate;
    }

    float sampleRate;
    float leftOutput;
    float rightOutput;
    bool isUsed;
    int nextChorusMode;
    
    RingBuffer ringBuffer;
    SimpleSinglePoleFilter preFilter;
    SimpleSinglePoleFilter postLeftFilter;
    SimpleSinglePoleFilter postRightFilter;
    
    float dryCurrent;
    float dryChange;
    float dryTarget;
    float lfoValue;
    float lfoIncrement;
    float maxLeftOffset;
    float averageLeftSamples;
    float maxRightOffset;
    float averageRightSamples;
};

} // namespace junox
