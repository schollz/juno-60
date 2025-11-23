Juno60 : MultiOutUGen {
    *ar { arg gate=0, freq=60, amp=0.5,
          attack=0, decay=0.5, sustain=0.5, release=0.5,
          cutoff=0.5, resonance=0, envMod=0,
          saw=1, pulse=0, sub=0, noise=0, pwm=0.5,
          chorus=0;
        
        ^this.multiNew('audio', gate, freq, amp,
            attack, decay, sustain, release,
            cutoff, resonance, envMod,
            saw, pulse, sub, noise, pwm,
            chorus)
    }
    
    *kr { arg gate=0, freq=60, amp=0.5,
          attack=0, decay=0.5, sustain=0.5, release=0.5,
          cutoff=0.5, resonance=0, envMod=0,
          saw=1, pulse=0, sub=0, noise=0, pwm=0.5,
          chorus=0;
        
        ^this.multiNew('control', gate, freq, amp,
            attack, decay, sustain, release,
            cutoff, resonance, envMod,
            saw, pulse, sub, noise, pwm,
            chorus)
    }
    
    checkInputs {
        ^this.checkValidInputs;
    }
    
    init { arg ... theInputs;
        inputs = theInputs;
        ^this.initOutputs(2, rate);
    }
}
