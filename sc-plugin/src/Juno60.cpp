#include "SC_PlugIn.hpp"
#include "Junox.hpp"

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
        
        synth = new junox::Junox(patch, sr, 6);
        
        mCalcFunc = make_calc_function<Juno60, &Juno60::next>();
        next(1);
    }

    ~Juno60() {
        delete synth;
    }

private:
    void next(int nSamples) {
        // Get input parameters
        const float* gate = in(0);
        const float* freq = in(1);
        const float* amp = in(2);
        
        // Attack, Decay, Sustain, Release
        const float attack = in0(3);
        const float decay = in0(4);
        const float sustain = in0(5);
        const float release = in0(6);
        
        // Filter parameters
        const float cutoff = in0(7);
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
        
        patch.chorus.mode = static_cast<int>(chorusMode);
        
        synth->update();
        
        // Process gate and trigger notes
        for (int i = 0; i < nSamples; i++) {
            if (gate[i] > 0.5f && mPrevGate <= 0.5f) {
                // Note on
                int note = static_cast<int>(freq[i]);
                if (note < 0) note = 0;
                if (note > 127) note = 127;
                float velocity = amp[i];
                if (velocity < 0.0f) velocity = 0.0f;
                if (velocity > 1.0f) velocity = 1.0f;
                synth->noteOn(note, velocity);
            } else if (gate[i] <= 0.5f && mPrevGate > 0.5f) {
                // Note off
                int note = static_cast<int>(freq[i]);
                if (note < 0) note = 0;
                if (note > 127) note = 127;
                synth->noteOff(note);
            }
            mPrevGate = gate[i];
        }
        
        // Render audio
        synth->render(outL, outR, nSamples);
    }

    junox::Junox* synth;
    float mPrevGate = 0.0f;
};

} // namespace Juno60

PluginLoad(Juno60UGens) {
    ft = inTable;
    registerUnit<Juno60::Juno60>(ft, "Juno60");
}
