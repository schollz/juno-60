#pragma once

#include "Juno60DCO.hpp"
#include "Juno60Envelope.hpp"
#include "LadderFilter.hpp"
#include "Noise.hpp"
#include <string>

namespace junox {

// Patch structure for voice configuration
struct Patch {
    struct DCO {
        bool saw = false;
        bool pulse = false;
        float subAmount = 0.0f;
        float noise = 0.0f;
        float lfo = 0.0f;
        float pwm = 0.0f;
        std::string pwmMod = ""; // "l" for LFO, "e" for envelope
        float range = 1.0f;
    } dco;

    struct VCF {
        float frequency = 0.5f;
        float resonance = 0.0f;
        float envMod = 0.0f;
        float lfoMod = 0.0f;
        float keyMod = 0.0f;
        bool modPositive = true;
    } vcf;

    struct ENV {
        float attack = 0.0f;
        float decay = 0.5f;
        float sustain = 0.5f;
        float release = 0.5f;
    } env;

    std::string vcaType = "env"; // "env" or "gate"
    float hpf = 0.0f;
    float vca = 0.5f;

    struct LFOConfig {
        float frequency = 0.5f;
        float delay = 0.0f;
        bool autoTrigger = false;
    } lfo;

    struct ChorusConfig {
        int mode = 0; // 0=off, 1=I, 2=II, 3=I+II
    } chorus;
};

/**
 * Single voice of the Juno-60 synthesizer.
 */
class Voice {
public:
    Voice(const Patch& patch, float sampleRate)
        : patch(patch)
        , sampleRate(sampleRate)
        , note(-1)
        , velocity(0.0f)
        , filterNoteFactor(0.0f)
        , dco(sampleRate)
        , noise(sampleRate, 5000.0f)
        , modEnv(sampleRate)
        , ampEnv(sampleRate)
        , moogVCF(sampleRate)
    {
    }

    float render(float lfoOut, float detuneFactor, float pwmDepth,
                 float sawLevel, float pulseLevel, float subLevel, float noiseLevel,
                 float filterCutoff, float filterResonance, float filterEnvMod,
                 float lfoDetuneOctaves, float filterKeyMod) {
        const float modEnvOut = modEnv.render();
        const float ampEnvOut = ampEnv.render();

        // Calculate pulse width modulation
        float pulseWidth = pwmDepth;
        if (patch.dco.pwmMod == "l") {
            pulseWidth *= lfoOut * 0.5f + 0.5f;
        } else if (patch.dco.pwmMod == "e") {
            pulseWidth *= modEnvOut;
        }

        // Render DCO
        float dcoOut = dco.render(detuneFactor, pulseWidth, sawLevel, pulseLevel, subLevel);
        if (noiseLevel > 0.0f) {
            dcoOut += noise.render() * noiseLevel;
        }

        // Calculate VCF cutoff with modulation
        const float cutoffDetuneOctave = (filterCutoff * 200.0f) / 12.0f;
        const float envDetuneOctaves = modEnvOut * filterEnvMod * 12.0f;
        const float keyboardDetuneOctaves = filterKeyMod * filterNoteFactor;
        const float resonanceDetuneOctaves = patch.vcf.resonance * 0.5f;
        const float vcfCutoffValue = 
            cutoffDetuneOctave +
            lfoDetuneOctaves * ampEnvOut +
            keyboardDetuneOctaves +
            envDetuneOctaves +
            resonanceDetuneOctaves;

        const float cutoffFrequency = 7.8f * std::pow(2.0f, vcfCutoffValue);
        const float vcfOut = moogVCF.process(dcoOut, moogVCF.calcCutoffFactor(cutoffFrequency), filterResonance);

        return velocity * vcfOut * ampEnvOut;
    }

    void noteOn(int noteNum, float vel) {
        if (noteNum != note || isFinished()) {
            note = noteNum;
            dco.noteOn(note);
            modEnv.reset();
            ampEnv.reset();
            moogVCF.reset();

            const int c4 = 60;
            const int fiveOctaves = 5 * 12;
            filterNoteFactor = 5.0f * ((note - c4) / (float)fiveOctaves);
        }

        // If no sound source, use filter as source
        if (!patch.dco.saw && !patch.dco.pulse && !patch.dco.subAmount && !patch.dco.noise) {
            const float initialExcite = patch.vcf.resonance * patch.vcf.resonance * 0.01f;
            moogVCF.trigger(initialExcite);
        }

        velocity = vel;
        updatePatch(patch);
        modEnv.trigger();
        ampEnv.trigger();
    }

    void noteOff() {
        modEnv.release();
        ampEnv.release();
    }

    bool isFinished() const {
        return ampEnv.isFinished();
    }

    void updatePatch(const Patch& p) {
        patch = p;
        
        modEnv.setValues(
            patch.env.attack,
            patch.env.decay,
            patch.env.sustain,
            patch.env.release
        );

        if (patch.vcaType == "env") {
            ampEnv.setValues(
                patch.env.attack,
                patch.env.decay,
                patch.env.sustain,
                patch.env.release
            );
        } else {
            ampEnv.setValues(0.00247f, 0.0057f, 0.98f, 0.0057f);
        }
    }

    int getNote() const { return note; }

private:
    Patch patch;
    float sampleRate;
    int note;
    float velocity;
    float filterNoteFactor;
    
    Juno60DCO dco;
    Noise noise;
    Juno60Envelope modEnv;
    Juno60Envelope ampEnv;
    LadderFilter moogVCF;
};

} // namespace junox
