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
- `attack`: Attack time (0-1)
- `decay`: Decay time (0-1)
- `sustain`: Sustain level (0-1)
- `release`: Release time (0-1)
- `cutoff`: Filter cutoff frequency (0-1)
- `resonance`: Filter resonance (0-1)
- `envMod`: Envelope modulation depth (0-1)
- `saw`: Sawtooth wave on/off (0 or 1)
- `pulse`: Pulse wave on/off (0 or 1)
- `sub`: Sub-oscillator level (0-1)
- `noise`: Noise level (0-1)
- `pwm`: Pulse width modulation (0-1)
- `chorus`: Chorus mode (0=off, 1=I, 2=II, 3=I+II)
- `doneAction`: DoneAction to perform when the release segment finishes (0=none, 2=free synth, etc.)

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

## License

GPL-3.0-or-later (same as original junox)
