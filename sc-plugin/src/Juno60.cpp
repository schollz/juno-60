#include "SC_PlugIn.hpp"
#include "Junox.hpp"
#include <algorithm>
#include <cmath>
#include <string>

static InterfaceTable* ft;

namespace Juno60 {

class Juno60 : public SCUnit {
public:
    Juno60() {
        const float sr = static_cast<float>(sampleRate());
        
        // Initialize default patch
        junox::Patch patch;
        patch.dco.saw = true;
        patch.dco.pulse = false;
        patch.dco.subAmount = 0.0f;
        patch.dco.noise = 0.0f;
        patch.dco.lfo = 0.0f;
        patch.dco.pwm = 0.5f;
        patch.dco.pwmMod = "";
        patch.dco.range = 1.0f;
        
        patch.vcf.frequency = 0.5f;
        patch.vcf.resonance = 0.0f;
        patch.vcf.envMod = 0.0f;
        patch.vcf.lfoMod = 0.0f;
        patch.vcf.keyMod = 0.0f;
        patch.vcf.modPositive = true;
        
        patch.env.attack = 0.0f;
        patch.env.decay = 0.5f;
        patch.env.sustain = 0.5f;
        patch.env.release = 0.5f;
        
        patch.vcaType = "env";
        patch.hpf = 0.0f;
        patch.vca = 0.5f;
        
        patch.lfo.frequency = 0.5f;
        patch.lfo.delay = 0.0f;
        patch.lfo.autoTrigger = false;
        
        patch.chorus.mode = 0;
        
        synth = new junox::Junox(patch, sr, 1);
        
        mCalcFunc = make_calc_function<Juno60, &Juno60::next>();
        next(1);
    }

    ~Juno60() {
        delete synth;
    }

private:
    static std::string pwmModFromParam(float value) {
        const int mode = static_cast<int>(std::round(value));
        switch (mode) {
        case 1:
            return "l";
        case 2:
            return "m";
        case 3:
            return "e";
        default:
            return "";
        }
    }

    void next(int nSamples) {
        // Get input parameters
        const float* gateIn = in(0);
        const float* freqIn = in(1);
        const float* ampIn = in(2);
        const bool gateIsAudioRate = inRate(0) == calc_FullRate;
        const bool noteIsAudioRate = inRate(1) == calc_FullRate;
        const bool ampIsAudioRate = inRate(2) == calc_FullRate;
        const float gateValue = gateIn[0];
        const float noteValue = freqIn[0];
        const float ampValue = ampIn[0];
        const int doneAction = static_cast<int>(in0(16));
        
        // Attack, Decay, Sustain, Release
        const float attack = in0(3);
        const float decay = in0(4);
        const float sustain = in0(5);
        const float release = in0(6);
        
        // Filter parameters
        const float cutoffInput = in0(7);
        float cutoff = cutoffInput;
        if (cutoffInput > 1.0f) {
            const float minFreq = 7.8f;
            float freqHz = cutoffInput;
            if (freqHz < minFreq) freqHz = minFreq;
            const float semitoneRange = 200.0f;
            const float normalized = (std::log2(freqHz / minFreq) * 12.0f) / semitoneRange;
            cutoff = std::max(0.0f, std::min(1.0f, normalized));
        }
        const float resonance = in0(8);
        const float envMod = in0(9);
        
        // DCO parameters
        const float sawOn = in0(10);
        const float pulseOn = in0(11);
        const float subLevel = in0(12);
        const float noiseLevel = in0(13);
        const float pwm = in0(14);
        
        // Chorus mode
        const float chorusMode = in0(15);
        
        // Extended parameters
        const float lfoRate = in0(17);
        const float lfoDelay = in0(18);
        const float lfoAuto = in0(19);
        const float dcoLfo = in0(20);
        const float dcoPwmMod = in0(21);
        const float dcoRange = in0(22);
        const float hpfAmount = in0(23);
        const float vcfLfo = in0(24);
        const float vcfKey = in0(25);
        const float vcfDir = in0(26);
        const float vcaTypeInput = in0(27);
        const float vcaValue = in0(28);
        const float chorusDrywet = in0(29);

        float* outL = out(0);
        float* outR = out(1);
        
        // Update patch from parameters once per buffer
        junox::Patch& patch = synth->getPatch();
        patch.env.attack = attack;
        patch.env.decay = decay;
        patch.env.sustain = sustain;
        patch.env.release = release;
        
        patch.vcf.frequency = cutoff;
        patch.vcf.resonance = resonance;
        patch.vcf.envMod = envMod;
        
        patch.dco.saw = sawOn > 0.5f;
        patch.dco.pulse = pulseOn > 0.5f;
        patch.dco.subAmount = subLevel;
        patch.dco.noise = noiseLevel;
        patch.dco.pwm = pwm;
        patch.dco.lfo = dcoLfo;
        patch.dco.pwmMod = pwmModFromParam(dcoPwmMod);
        patch.dco.range = (dcoRange <= 0.0f) ? 1.0f : dcoRange;
        
        patch.chorus.mode = static_cast<int>(chorusMode);
        patch.chorus.drywet = std::max(0.0f, std::min(1.0f, chorusDrywet));
        patch.lfo.frequency = lfoRate;
        patch.lfo.delay = lfoDelay;
        patch.lfo.autoTrigger = lfoAuto > 0.5f;
        
        patch.hpf = std::max(0.0f, std::min(1.0f, hpfAmount));
        patch.vcf.lfoMod = vcfLfo;
        patch.vcf.keyMod = vcfKey;
        patch.vcf.modPositive = vcfDir >= 0.5f;
        
        patch.vcaType = vcaTypeInput >= 0.5f ? "gate" : "env";
        patch.vca = vcaValue;
        
        synth->update();
        
        // Process gate and trigger notes
        for (int i = 0; i < nSamples; i++) {
            const float gateSample = gateIsAudioRate ? gateIn[i] : gateValue;
            const float noteSample = noteIsAudioRate ? freqIn[i] : noteValue;
            const float ampSample = ampIsAudioRate ? ampIn[i] : ampValue;
            int desiredNote = static_cast<int>(noteSample);
            if (desiredNote < 0) desiredNote = 0;
            if (desiredNote > 127) desiredNote = 127;
            float velocity = ampSample;
            if (velocity < 0.0f) velocity = 0.0f;
            if (velocity > 1.0f) velocity = 1.0f;

            if (gateSample > 0.5f && mPrevGate <= 0.5f) {
                // Gate on -> start note
                synth->noteOn(desiredNote, velocity);
                mActiveNote = desiredNote;
                mReleasePending = false;
                mPendingDoneAction = 0;
            } else if (gateSample > 0.5f && mPrevGate > 0.5f) {
                // Gate held: allow note changes for mono playing
                if (desiredNote != mActiveNote && mActiveNote >= 0) {
                    synth->noteOff(mActiveNote);
                    synth->noteOn(desiredNote, velocity);
                    mActiveNote = desiredNote;
                }
            } else if (gateSample <= 0.5f && mPrevGate > 0.5f) {
                // Gate released -> note off
                synth->releaseAll();
                mActiveNote = -1;
                mPendingDoneAction = doneAction;
                mReleasePending = (mPendingDoneAction > 0);
            }
            mPrevGate = gateSample;
        }
        
        // Render audio
        synth->render(outL, outR, nSamples);

        if (mReleasePending && mPendingDoneAction > 0 && !synth->isActive()) {
            DoneAction(mPendingDoneAction, this);
            mReleasePending = false;
            mPendingDoneAction = 0;
        }
    }

    junox::Junox* synth;
    float mPrevGate = 0.0f;
    int mActiveNote = -1;
    int mPendingDoneAction = 0;
    bool mReleasePending = false;
};

} // namespace Juno60

PluginLoad(Juno60UGens) {
    ft = inTable;
    registerUnit<Juno60::Juno60>(ft, "Juno60");
}
