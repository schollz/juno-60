#pragma once

#include <cmath>
#include <algorithm>
#include "Utils.hpp"

namespace junox {

/**
 * Juno-60 ADSR envelope.
 */
class Juno60Envelope {
public:
    Juno60Envelope(float sampleRate)
        : sampleRate(sampleRate)
        , currentLevel(0.0f)
        , attackDuration(0.001f)
        , decayDuration(0.025f)
        , sustainLevel(0.0f)
        , releaseDuration(0.025f)
        , state(State::IDLE)
        , sampleCount(0)
    {
    }

    void setValues(float attack, float decay, float sustain, float release) {
        attackDuration = attack;
        decayDuration = decay;
        sustainLevel = std::max(0.02f, sustain);
        releaseDuration = (sustainLevel <= 0.02f) ? 0.01f : release;
    }

    void setValuesFromSliders(float attackSlider, float decaySlider, 
                              float sustainSlider, float releaseSlider) {
        // Curves from the Juno-60 service manual
        static const float curveFromAttackSliderToDuration[] = {0.001f, 0.03f, 0.24f, 0.65f, 3.25f};
        static const float curveFromDecaySliderToDuration[] = {0.002f, 0.096f, 0.984f, 4.449f, 19.783f};
        static const float curveFromReleaseSliderToDuration[] = {0.002f, 0.096f, 0.984f, 4.449f, 19.783f};

        const float attackDur = interpolatedLookup(attackSlider * 5.0f, curveFromAttackSliderToDuration, 5);
        const float decayDur = interpolatedLookup(decaySlider * 5.0f, curveFromDecaySliderToDuration, 5);
        const float releaseDur = interpolatedLookup(releaseSlider * 5.0f, curveFromReleaseSliderToDuration, 5);

        setValues(attackDur, decayDur, sustainSlider, releaseDur);
    }

    void trigger() {
        state = State::ATTACK;
        sampleCount = 0;
    }

    void release() {
        state = State::RELEASE;
        sampleCount = 0;
    }

    void reset() {
        currentLevel = 0.0f;
        state = State::IDLE;
        sampleCount = 0;
    }

    bool isFinished() const {
        return state == State::IDLE;
    }

    float render() {
        switch (state) {
            case State::IDLE:
                currentLevel = 0.0f;
                break;

            case State::ATTACK: {
                const int attackSamples = static_cast<int>(attackDuration * sampleRate);
                if (attackSamples > 0) {
                    // Exponential attack curve (targeting 0.632 of final value per time constant)
                    const float target = 1.0f;
                    const float attackRate = 1.0f / attackSamples;
                    currentLevel += (target - currentLevel) * attackRate * 2.5f;
                    sampleCount++;
                    if (sampleCount >= attackSamples || currentLevel >= 0.99f) {
                        currentLevel = 1.0f;
                        state = State::DECAY;
                        sampleCount = 0;
                    }
                } else {
                    currentLevel = 1.0f;
                    state = State::DECAY;
                    sampleCount = 0;
                }
                break;
            }

            case State::DECAY: {
                const int decaySamples = static_cast<int>(decayDuration * sampleRate);
                if (decaySamples > 0 && currentLevel > sustainLevel) {
                    // Exponential decay
                    const float decayRate = 1.0f / decaySamples;
                    currentLevel += (sustainLevel - currentLevel) * decayRate * 5.0f;
                    sampleCount++;
                    if (sampleCount >= decaySamples || currentLevel <= sustainLevel * 1.01f) {
                        currentLevel = sustainLevel;
                        state = State::SUSTAIN;
                    }
                } else {
                    currentLevel = sustainLevel;
                    state = State::SUSTAIN;
                }
                break;
            }

            case State::SUSTAIN:
                currentLevel = sustainLevel;
                break;

            case State::RELEASE: {
                const int releaseSamples = static_cast<int>(releaseDuration * sampleRate);
                if (releaseSamples > 0 && currentLevel > 0.0001f) {
                    // Exponential release
                    const float releaseRate = 1.0f / releaseSamples;
                    currentLevel *= (1.0f - releaseRate * 5.0f);
                    if (currentLevel < 0.0001f) {
                        currentLevel = 0.0f;
                        state = State::IDLE;
                    }
                } else {
                    currentLevel = 0.0f;
                    state = State::IDLE;
                }
                break;
            }
        }

        return currentLevel;
    }

private:
    enum class State {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };

    float sampleRate;
    float currentLevel;
    float attackDuration;
    float decayDuration;
    float sustainLevel;
    float releaseDuration;
    State state;
    int sampleCount;
};

} // namespace junox
