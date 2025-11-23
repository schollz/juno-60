#pragma once

#include "RingBuffer.hpp"
#include "SimpleSinglePoleFilter.hpp"
#include <cmath>
#include <algorithm>

namespace junox {

/**
 * Roland Juno-60 chorus effect emulation.
 * Supports original modes (1=I, 2=II, 3=I+II) and junologue modes (4=I, 5=I-II, 6=II).
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
        , lfo2Value(0.0f)
        , lfo2Increment(0.01f)
        , maxLeftOffset(0.0f)
        , averageLeftSamples(0.0f)
        , maxRightOffset(0.0f)
        , averageRightSamples(0.0f)
        , isJunologueMode(false)
        , junologueLeftGain(0.0f)
        , junologueRightGain(0.0f)
        , wetDryMix(1.0f)
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
                update(nextChorusMode, wetDryMix);
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

        if (isJunologueMode) {
            renderJunologue(input, dry);
        } else {
            renderOriginal(input, dry);
        }
    }

    void renderOriginal(float input, float dry) {
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

    void renderJunologue(float input, float dry) {
        // Update both LFOs (triangle wave)
        float lfo1Val = lfoValue + lfoIncrement;
        if (lfo1Val > 1.0f || lfo1Val < -1.0f) {
            lfoIncrement = -lfoIncrement;
            lfo1Val = lfoValue + lfoIncrement;
        }
        lfoValue = lfo1Val;

        float lfo2Val = lfo2Value + lfo2Increment;
        if (lfo2Val > 1.0f || lfo2Val < -1.0f) {
            lfo2Increment = -lfo2Increment;
            lfo2Val = lfo2Value + lfo2Increment;
        }
        lfo2Value = lfo2Val;

        // Add input to ring buffer with filtering and soft saturation
        ringBuffer.writeSample(preFilter.renderLP(applySaturation(input)));

        // Read delays with dual-LFO modulation
        const float leftDelay1 = averageLeftSamples + lfo1Val * maxLeftOffset;
        const float leftDelay2 = averageRightSamples + lfo2Val * maxRightOffset;

        const float leftWet1 = ringBuffer.readSample(leftDelay1) * junologueLeftGain;
        const float leftWet2 = ringBuffer.readSample(leftDelay2) * junologueRightGain;
        const float leftWet = postLeftFilter.renderLP(leftWet1 + leftWet2);

        const float rightDelay1 = averageLeftSamples + (1.0f - lfo1Val) * maxLeftOffset;
        const float rightDelay2 = averageRightSamples + (1.0f - lfo2Val) * maxRightOffset;

        const float rightWet1 = ringBuffer.readSample(rightDelay1) * junologueLeftGain;
        const float rightWet2 = ringBuffer.readSample(rightDelay2) * junologueRightGain;
        const float rightWet = postRightFilter.renderLP(rightWet1 + rightWet2);

        // Mix dry and wet
        const float dryGain = std::sqrt(dry);
        const float wetGain = std::sqrt(1.0f - dry);

        leftOutput = input * dryGain + leftWet * wetGain;
        rightOutput = input * dryGain + rightWet * wetGain;
    }

    void reset() {
        ringBuffer.reset();
        preFilter.reset();
        postLeftFilter.reset();
        postRightFilter.reset();
        isUsed = false;
    }

    void update(int chorusMode, float drywet = 1.0f) {
        wetDryMix = drywet;

        if (dryCurrent < 1.0f && !isUsed) {
            // Avoid clicks - transition to fully dry first
            dryChange = 0.0005f;
            dryTarget = 1.0f;
            nextChorusMode = chorusMode;
        } else {
            switch (chorusMode) {
                case 1: // Mode I
                    isJunologueMode = false;
                    updateValues(0.513f, 1.0f - wetDryMix * 0.56f, 0.00154f, 0.00515f, 0.00151f, 0.0054f, true);
                    break;
                case 2: // Mode II
                    isJunologueMode = false;
                    updateValues(0.863f, 1.0f - wetDryMix * 0.56f, 0.00154f, 0.00515f, 0.00151f, 0.0054f, true);
                    break;
                case 3: // Mode I+II
                    isJunologueMode = false;
                    updateValues(9.75f, 1.0f - wetDryMix * 0.56f, 0.00322f, 0.00356f, 0.00328f, 0.00365f, false);
                    break;
                case 4: // Mode I (junologue) - left only
                    isJunologueMode = true;
                    updateJunologueValues(1.0f - wetDryMix, 1.0f, 0.0f);
                    break;
                case 5: // Mode I-II (junologue) - both
                    isJunologueMode = true;
                    updateJunologueValues(1.0f - wetDryMix, 0.707107f, 0.707107f);
                    break;
                case 6: // Mode II (junologue) - right only
                    isJunologueMode = true;
                    updateJunologueValues(1.0f - wetDryMix, 0.0f, 1.0f);
                    break;
                default: // Off
                    isJunologueMode = false;
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

    void updateJunologueValues(float dry, float leftGain, float rightGain) {
        // Junologue delay times (in seconds)
        const float minDelay1 = 0.00154f;
        const float maxDelay1 = 0.00515f;
        const float minDelay2 = 0.00151f;
        const float maxDelay2 = 0.0054f;

        // Left channel uses delay 1
        const float avgDelay1 = (minDelay1 + maxDelay1) * 0.5f;
        averageLeftSamples = avgDelay1 * sampleRate;
        maxLeftOffset = (maxDelay1 - avgDelay1) * sampleRate;

        // Right channel uses delay 2
        const float avgDelay2 = (minDelay2 + maxDelay2) * 0.5f;
        averageRightSamples = avgDelay2 * sampleRate;
        maxRightOffset = (maxDelay2 - avgDelay2) * sampleRate;

        // Set delay gains (equivalent to delay_gains array)
        junologueLeftGain = leftGain;
        junologueRightGain = rightGain;

        // Transition to desired wet/dry ratio
        dryTarget = dry;
        if (!isUsed) {
            dryCurrent = dry;
        }
        dryChange = (dry - dryCurrent) / 1000.0f;

        // LFO increments for junologue rates: 0.513 Hz and 0.863 Hz
        lfoIncrement = (lfoIncrement >= 0.0f ? 1.0f : -1.0f) * 4.0f * 0.513f / sampleRate;
        lfo2Increment = (lfo2Increment >= 0.0f ? 1.0f : -1.0f) * 4.0f * 0.863f / sampleRate;
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
    float lfo2Value;
    float lfo2Increment;
    float maxLeftOffset;
    float averageLeftSamples;
    float maxRightOffset;
    float averageRightSamples;
    bool isJunologueMode;
    float junologueLeftGain;
    float junologueRightGain;
    float wetDryMix;
};

} // namespace junox
