#pragma once

#include "Voice.hpp"
#include "Chorus.hpp"
#include "LFOWithEnvelope.hpp"
#include "SimpleSinglePoleFilter.hpp"
#include "SmoothMoves.hpp"
#include "Utils.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace junox {

/**
 * Main Juno-60 synthesizer class.
 */
class Junox {
public:
    Junox(const Patch& patch, float sampleRate, int polyphony = 6)
        : patch(patch)
        , sampleRate(sampleRate)
        , maxVoices(polyphony)
        , status(0)
        , bendAmountParam(0.0f, sampleRate)
        , dcoBendDepthParam(1.0f, sampleRate)
        , pitchLfoModDepthParam(0.0f, sampleRate)
        , pwmDepthParam(0.0f, sampleRate)
        , sawLevelParam(0.0f, sampleRate)
        , pulseLevelParam(0.0f, sampleRate)
        , subLevelParam(0.0f, sampleRate)
        , noiseLevelParam(0.0f, sampleRate)
        , filterCutoffParam(0.0f, sampleRate)
        , filterResonanceParam(0.0f, sampleRate)
        , filterBendDepthParam(1.0f, sampleRate)
        , filterEnvModParam(0.0f, sampleRate)
        , filterLfoModParam(0.0f, sampleRate)
        , filterKeyModParam(0.0f, sampleRate)
        , vcaGainFactorParam(0.0f, sampleRate)
        , lfo(sampleRate)
        , hpf(sampleRate)
        , chorus(sampleRate)
    {
        lfo.setWaveform(LFO::Waveform::SINE);
        update();
    }

    void noteOn(int note, float velocity) {
        status = 4; // NOTES_ACTIVE

        // If note already playing, retrigger
        for (auto& voice : voices) {
            if (voice->getNote() == note) {
                voice->noteOn(note, velocity);
                return;
            }
        }

        // Trigger LFO if needed
        if (voices.empty() && patch.lfo.autoTrigger) {
            lfo.trigger();
        }

        auto newVoice = std::make_unique<Voice>(patch, sampleRate);
        newVoice->noteOn(note, velocity);

        if (voices.size() < (size_t)maxVoices) {
            voices.push_back(std::move(newVoice));
        } else {
            // Replace first voice (simple voice stealing)
            voices[0] = std::move(newVoice);
        }
    }

    void noteOff(int note) {
        for (auto& voice : voices) {
            if (voice->getNote() == note && !voice->isFinished()) {
                voice->noteOff();
            }
        }
    }

    void pitchBend(float value) {
        bendAmountParam.setValue(value);
    }

    void lfoTrigger() {
        lfo.trigger();
    }

    void lfoRelease() {
        lfo.release();
    }

    void render(float* outL, float* outR, int numSamples) {
        // If silent, return immediately
        if (status == 0) {
            return;
        }
        status--;

        // Remove finished voices
        voices.erase(
            std::remove_if(voices.begin(), voices.end(),
                [](const std::unique_ptr<Voice>& v) { return v->isFinished(); }),
            voices.end()
        );

        if (!voices.empty()) {
            status = 4; // NOTES_ACTIVE
        }

        // Render samples
        for (int i = 0; i < numSamples; i++) {
            const float bendAmount = bendAmountParam.getNextValue();
            const float dcoBendDepth = dcoBendDepthParam.getNextValue();
            const float pwmDepth = pwmDepthParam.getNextValue();
            const float pitchLfoModDepth = pitchLfoModDepthParam.getNextValue();
            const float sawLevel = sawLevelParam.getNextValue();
            const float pulseLevel = pulseLevelParam.getNextValue();
            const float subLevel = subLevelParam.getNextValue();
            const float noiseLevel = noiseLevelParam.getNextValue();
            const float filterCutoff = filterCutoffParam.getNextValue();
            const float filterResonance = filterResonanceParam.getNextValue();
            const float filterBendDepth = filterBendDepthParam.getNextValue();
            const float filterEnvMod = filterEnvModParam.getNextValue();
            const float filterLfoMod = filterLfoModParam.getNextValue();
            const float filterKeyMod = filterKeyModParam.getNextValue();
            const float vcaGainFactor = vcaGainFactorParam.getNextValue();

            const float lfoOut = lfo.render();

            // Calculate detuning
            const float dcoDetuneOctaves = 
                lfoOut * pitchLfoModDepth * 0.25f +
                (bendAmount * dcoBendDepth * 7.0f) / 12.0f;
            float dcoDetuneFactor = patch.dco.range;
            if (dcoDetuneOctaves != 0.0f) {
                dcoDetuneFactor *= std::pow(2.0f, dcoDetuneOctaves);
            }
            const float filterDetuneOctaves = 
                bendAmount * filterBendDepth * 4.0f +
                filterLfoMod * lfoOut * 3.0f;

            // Gather voice outputs
            float monoOut = 0.0f;
            for (auto& voice : voices) {
                if (!voice->isFinished()) {
                    monoOut += voice->render(
                        lfoOut, dcoDetuneFactor, pwmDepth,
                        sawLevel, pulseLevel, subLevel, noiseLevel,
                        filterCutoff, filterResonance, filterEnvMod,
                        filterDetuneOctaves, filterKeyMod
                    );
                }
            }

            // Apply high-pass filter
            if (patch.hpf > 0.0f) {
                float lowPassOut = hpf.renderLP(monoOut);
                if (patch.hpf < 0.25f) {
                    lowPassOut *= patch.hpf * 4.0f;
                }
                monoOut -= lowPassOut;
            }

            // Apply VCA gain
            monoOut *= vcaGainFactor;

            // Soft clip
            monoOut = fastTanh(3.0f * monoOut);

            // Apply chorus
            chorus.render(monoOut);
            outL[i] = chorus.getLeftOutput();
            outR[i] = chorus.getRightOutput();
        }

        // Check if synth should now be silent
        if (status == 0) {
            // Fade out
            float fadeLevel = 1.0f;
            const float fadeStep = fadeLevel / numSamples;
            for (int i = 0; i < numSamples; i++) {
                outL[i] *= fadeLevel;
                outR[i] *= fadeLevel;
                fadeLevel -= fadeStep;
            }

            // Reset stateful elements
            if (patch.lfo.autoTrigger) {
                lfo.reset();
            }
            hpf.reset();
            chorus.reset();

            // Reset parameters
            bendAmountParam.reset();
            dcoBendDepthParam.reset();
            pitchLfoModDepthParam.reset();
            pwmDepthParam.reset();
            sawLevelParam.reset();
            pulseLevelParam.reset();
            subLevelParam.reset();
            noiseLevelParam.reset();
            filterCutoffParam.reset();
            filterResonanceParam.reset();
            filterBendDepthParam.reset();
            filterEnvModParam.reset();
            filterLfoModParam.reset();
            filterKeyModParam.reset();
            vcaGainFactorParam.reset();
        }
    }

    void setValue(const std::string& path, float value) {
        // Simple path parsing for patch updates
        // In a full implementation, this would be more sophisticated
        update();
    }

    void update() {
        bool isActive = false;
        for (auto& voice : voices) {
            voice->updatePatch(patch);
            isActive = isActive || !voice->isFinished();
        }

        // Calculate source levels
        float sawLevel = patch.dco.saw ? 0.2f : 0.0f;
        float pulseLevel = patch.dco.pulse ? 0.2f : 0.0f;
        float subLevel = patch.dco.subAmount * 0.195f;
        float noiseLevel = patch.dco.noise * 0.21f;

        // Adjust mix if multiple sources
        float mixFactor = sawLevel + pulseLevel + subLevel + noiseLevel;
        if (mixFactor > 0.26f) {
            mixFactor = 0.26f / (0.26f + (mixFactor - 0.26f) * 0.3f);
            sawLevel *= mixFactor;
            pulseLevel *= mixFactor;
            subLevel *= mixFactor;
            noiseLevel *= mixFactor;
        }

        sawLevelParam.setValue(sawLevel, isActive);
        pulseLevelParam.setValue(pulseLevel, isActive);
        subLevelParam.setValue(subLevel, isActive);
        noiseLevelParam.setValue(noiseLevel, isActive);
        pitchLfoModDepthParam.setValue(patch.dco.lfo, isActive);
        pwmDepthParam.setValue(patch.dco.pwm, isActive);

        const float envModDirection = patch.vcf.modPositive ? 1.0f : -1.0f;
        filterCutoffParam.setValue(patch.vcf.frequency, isActive);
        filterResonanceParam.setValue(patch.vcf.resonance, isActive);
        filterEnvModParam.setValue(patch.vcf.envMod * envModDirection, isActive);
        filterLfoModParam.setValue(patch.vcf.lfoMod, isActive);
        filterKeyModParam.setValue(patch.vcf.keyMod, isActive);

        chorus.update(patch.chorus.mode);
        setLfoValuesFromSliders(patch.lfo.frequency, patch.lfo.delay);
        setHpfValuesFromSliders(patch.hpf);

        // VCA gain
        const float vcaGainFactor = std::pow(1.2589f, patch.vca * 10.0f) * 0.1f;
        vcaGainFactorParam.setValue(vcaGainFactor, isActive);
    }

    void panic() {
        voices.clear();
    }

private:
    void setLfoValuesFromSliders(float rateSlider, float delaySlider) {
        static const float curveFromLfoRateSliderToFreq[] = {0.3f, 0.85f, 3.39f, 11.49f, 22.22f};
        static const float curveFromLfoDelaySliderToDelay[] = {0.0f, 0.0639f, 0.85f, 1.2f, 2.685f};
        static const float curveFromLfoDelaySliderToAttack[] = {0.001f, 0.053f, 0.188f, 0.348f, 1.15f};

        const float frequency = interpolatedLookup(rateSlider * 5.0f, curveFromLfoRateSliderToFreq, 5);
        const float delayDuration = interpolatedLookup(delaySlider * 5.0f, curveFromLfoDelaySliderToDelay, 5);
        const float attackDuration = interpolatedLookup(delaySlider * 5.0f, curveFromLfoDelaySliderToAttack, 5);

        lfo.setValues(frequency, delayDuration, attackDuration);
    }

    void setHpfValuesFromSliders(float rateSlider) {
        static const float curveFromHpfSliderToFreq[] = {140.0f, 250.0f, 520.0f, 1220.0f};
        const float frequency = interpolatedLookup(rateSlider * 4.0f, curveFromHpfSliderToFreq, 4);
        hpf.setCutoff(frequency);
    }

    Patch patch;
    float sampleRate;
    int maxVoices;
    int status;
    std::vector<std::unique_ptr<Voice>> voices;

    // Smoothed parameters
    SmoothMoves bendAmountParam;
    SmoothMoves dcoBendDepthParam;
    SmoothMoves pitchLfoModDepthParam;
    SmoothMoves pwmDepthParam;
    SmoothMoves sawLevelParam;
    SmoothMoves pulseLevelParam;
    SmoothMoves subLevelParam;
    SmoothMoves noiseLevelParam;
    SmoothMoves filterCutoffParam;
    SmoothMoves filterResonanceParam;
    SmoothMoves filterBendDepthParam;
    SmoothMoves filterEnvModParam;
    SmoothMoves filterLfoModParam;
    SmoothMoves filterKeyModParam;
    SmoothMoves vcaGainFactorParam;

    LFOWithEnvelope lfo;
    SimpleSinglePoleFilter hpf;
    Chorus chorus;
};

} // namespace junox
