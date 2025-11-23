# Juno60 SuperCollider Plugin

This is a C++ reimplementation of the junox synthesizer as a SuperCollider UGen plugin.

## Features

- Full Juno-60 synthesizer emulation in C++
- Multiple oscillator types (sawtooth, pulse, sub-oscillator)
- Moog-style ladder filter with resonance
- ADSR envelope generator
- LFO with multiple waveforms
- Authentic Juno-60 chorus effect
- Polyphonic (6 voices by default)

## Building

### Prerequisites

- CMake 3.5 or higher
- C++ compiler with C++14 support
- SuperCollider source code (automatically downloaded if not present)

### Build Instructions

1. Build the plugin:
   ```bash
   make
   ```

2. Install to SuperCollider Extensions folder:
   ```bash
   make install
   ```

3. Uninstall:
   ```bash
   make uninstall
   ```

### Custom SuperCollider Path

If you have SuperCollider source at a custom location:
```bash
make SC_PATH=/path/to/supercollider install
```

## Usage

### Parameters

- `gate`: Gate signal (>0.5 = on, <=0.5 = off)
- `freq`: MIDI note number (0-127)
- `amp`: Amplitude/velocity (0-1)
- `attack`: Attack time in seconds
- `decay`: Decay time in seconds
- `sustain`: Sustain level (0-1)
- `release`: Release time in seconds
- `cutoff`: Filter cutoff (0-1 for normalized slider, or >1 for frequency in Hz)
- `resonance`: Filter resonance (0-1)
- `envMod`: Envelope modulation depth (0-1)
- `saw`: Sawtooth wave level (0-1)
- `pulse`: Pulse wave level (0-1)
- `sub`: Sub-oscillator level (0-1)
- `noise`: Noise level (0-1)
- `pwm`: Pulse width modulation (0-1)
- `chorus`: Chorus mode (0=off, 1=I, 2=II, 3=I+II, 4=I(junologue), 5=I-II(junologue), 6=II(junologue))
- `doneAction`: DoneAction to perform when the release segment finishes (0=none, 2=free synth, etc.)
- `lfoRate`: LFO rate (Hz, normalized 0-1 slider equivalent)
- `lfoDelay`: LFO delay amount (0-1 slider equivalent)
- `lfoAuto`: LFO auto-trigger (0=manual, 1=auto)
- `dcoLfo`: Pitch LFO depth for the DCO (0-1)
- `dcoPwmMod`: PWM modulation source selector (0=none, 1=LFO, 2=manual, 3=ENV)
- `dcoRange`: Oscillator range multiplier (1.0 = normal)
- `hpf`: High-pass filter amount (0-1)
- `vcfLfo`: Filter LFO modulation depth (0-1)
- `vcfKey`: Filter keyboard tracking amount (0-1)
- `vcfDir`: Filter envelope polarity (0 = negative, 1 = positive)
- `vcaType`: VCA control source (0 = envelope, 1 = gate)
- `vcaValue`: VCA gain slider (roughly 0-1)
- `chorusDrywet`: Chorus dry/wet mix (0.0 = fully dry, 1.0 = fully wet)

### Example

```supercollider
// Simple sawtooth bass
{
    var gate = Impulse.kr(2);
    var sig = Juno60.ar(
        gate: gate,
        freq: 36,
        amp: 0.7,
        attack: 0.01,
        decay: 0.3,
        sustain: 0.2,
        release: 0.4,
        cutoff: 0.3,
        resonance: 0.5,
        envMod: 0.8,
        saw: 1
    );
    sig * 0.5;
}.play;
```

See `test_juno60.scd` for more examples.

## Architecture

The plugin is organized into modular C++ classes:

- `Junox`: Main synthesizer class
- `Voice`: Individual voice management
- `Juno60DCO`: Digital Controlled Oscillator with PolyBLEP
- `LadderFilter`: Moog-style 4-pole ladder filter
- `Juno60Envelope`: ADSR envelope generator
- `LFO`: Low-frequency oscillator
- `Chorus`: Juno-60 chorus effect
- `Noise`: Pink noise generator
- `SmoothMoves`: Parameter smoothing

## Attributions

This plugin is based on code from the following projects:

### JunoX
- **Repository**: https://github.com/pendragon-andyh/junox
- **Authors**: Originally by Daniele Zannotti, continued by Andy Harman (pendragon-andyh)
- **License**: GPL-3.0-or-later
- **Description**: JavaScript library that emulates the Roland Juno-60 synthesizer. The core synthesis engine (DCO, VCF, envelopes, LFO) is based on this implementation.

### Junologue Chorus
- **Repository**: https://github.com/peterall/junologue-chorus
- **Author**: Peter Allwin
- **License**: MIT License (Copyright 2020 Peter Allwin)
- **Description**: Juno-60 chorus emulation for Korg Prologue, Minilogue XD and Nu:tekt NTS-1. The junologue chorus modes (4=I, 5=I-II, 6=II) are based on this implementation.
- **Special Thanks**: Andy Harman and JP Cimalando for Juno 60 chorus research

## License

GPL-3.0-or-later

This plugin incorporates code from junologue-chorus (MIT License), which is compatible with the GPL-3.0-or-later license of this project.
