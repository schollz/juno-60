#pragma once

#include "LFO.hpp"

namespace junox {

/**
 * LFO with envelope (delay + attack).
 */
class LFOWithEnvelope {
public:
    LFOWithEnvelope(float sampleRate)
        : lfo(sampleRate)
        , sampleRate(sampleRate)
        , delayDuration(0.0f)
        , attackDuration(0.0f)
        , currentLevel(0.0f)
        , state(State::IDLE)
        , sampleCount(0)
    {
        lfo.setWaveform(LFO::Waveform::SINE);
    }

    void setValues(float frequency, float delay, float attack) {
        lfo.setRate(frequency);
        delayDuration = delay;
        attackDuration = attack;
    }

    void trigger() {
        state = State::DELAY;
        currentLevel = 0.0f;
        sampleCount = 0;
    }

    void release() {
        state = State::IDLE;
        currentLevel = 0.0f;
    }

    void reset() {
        lfo.reset();
        state = State::IDLE;
        currentLevel = 0.0f;
        sampleCount = 0;
    }

    float render() {
        const float lfoOut = lfo.render();

        switch (state) {
            case State::IDLE:
                currentLevel = 0.0f;
                break;

            case State::DELAY: {
                const int delaySamples = static_cast<int>(delayDuration * sampleRate);
                if (++sampleCount >= delaySamples) {
                    state = State::ATTACK;
                    sampleCount = 0;
                }
                currentLevel = 0.0f;
                break;
            }

            case State::ATTACK: {
                const int attackSamples = static_cast<int>(attackDuration * sampleRate);
                if (attackSamples > 0) {
                    currentLevel = std::min(1.0f, (float)sampleCount / attackSamples);
                    sampleCount++;
                    if (currentLevel >= 1.0f) {
                        state = State::SUSTAIN;
                    }
                } else {
                    currentLevel = 1.0f;
                    state = State::SUSTAIN;
                }
                break;
            }

            case State::SUSTAIN:
                currentLevel = 1.0f;
                break;
        }

        return lfoOut * currentLevel;
    }

    void setWaveform(LFO::Waveform wf) {
        lfo.setWaveform(wf);
    }

private:
    enum class State {
        IDLE,
        DELAY,
        ATTACK,
        SUSTAIN
    };

    LFO lfo;
    float sampleRate;
    float delayDuration;
    float attackDuration;
    float currentLevel;
    State state;
    int sampleCount;
};

} // namespace junox
