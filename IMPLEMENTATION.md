# Juno-60 SuperCollider Plugin Implementation

This document describes the C++ implementation of the junox synthesizer as a SuperCollider UGen plugin.

## Architecture

The implementation follows a modular architecture, with each component implemented as a separate C++ class:

### Core Audio Components

1. **Juno60DCO** (`Juno60DCO.hpp`)
   - Digital Controlled Oscillator with PolyBLEP anti-aliasing
   - Supports sawtooth, pulse, and sub-oscillator waveforms
   - PWM (Pulse Width Modulation) support
   - Note-accurate frequency generation (442Hz tuning)

2. **LadderFilter** (`LadderFilter.hpp`)
   - Moog-style Virtual Analog 4-pole ladder filter
   - Voltage-controlled cutoff frequency (1V/octave)
   - Resonance control with feedback

3. **Juno60Envelope** (`Juno60Envelope.hpp`)
   - ADSR envelope generator
   - Exponential attack, decay, and release curves
   - Juno-60 specific timing curves from service manual

4. **LFO** (`LFO.hpp`)
   - Multiple waveforms: sine, triangle, square, random, noise
   - Thread-safe random number generation using std::mt19937

5. **LFOWithEnvelope** (`LFOWithEnvelope.hpp`)
   - LFO with delay and attack phases
   - Auto-trigger support

6. **Chorus** (`Chorus.hpp`)
   - Authentic Juno-60 chorus effect
   - Three modes: I, II, and I+II
   - BBD (Bucket Brigade Device) emulation with delay lines

### Support Components

7. **Voice** (`Voice.hpp`)
   - Individual voice management
   - Combines DCO, filter, envelopes, and noise
   - Keyboard tracking for filter

8. **Junox** (`Junox.hpp`)
   - Main synthesizer class
   - Polyphonic voice management (6 voices)
   - Parameter smoothing
   - High-pass filter
   - Soft clipping

9. **Noise** (`Noise.hpp`)
   - Pink noise generator with single-pole filter

10. **SmoothMoves** (`SmoothMoves.hpp`)
    - Parameter smoothing to avoid zipper noise

11. **SimpleSinglePoleFilter** (`SimpleSinglePoleFilter.hpp`)
    - Basic low-pass filter for chorus and HPF

12. **RingBuffer** (`RingBuffer.hpp`)
    - Delay line for chorus effect

## SuperCollider Integration

### UGen Interface (`Juno60.cpp`)

The plugin exposes 16 control parameters:
- **gate**: Note on/off trigger
- **freq**: MIDI note number (0-127)
- **amp**: Velocity (0-1)
- **attack, decay, sustain, release**: Envelope parameters
- **cutoff, resonance, envMod**: Filter parameters
- **saw, pulse, sub, noise**: Oscillator mix
- **pwm**: Pulse width modulation
- **chorus**: Chorus mode (0-3)

### SuperCollider Class (`Juno60.sc`)

Provides a user-friendly interface with .ar and .kr constructors.

## Build System

### Makefile Features
- Automatic SuperCollider source download (Version-3.14.0)
- Git submodule initialization
- SC_PATH environment variable support
- Install/uninstall targets for Extensions folder
- Cross-platform support (Linux, macOS, Windows)

### Build Commands
```bash
# Build the plugin
make

# Install to SuperCollider Extensions
make install

# Uninstall
make uninstall

# Custom SuperCollider path
make SC_PATH=/path/to/sc install
```

## Testing

The `test_juno60.scd` file includes 6 example patches:
1. Basic sawtooth sound
2. Pulse wave with PWM
3. Mixed waveforms with chorus
4. Bass sound with sub oscillator
5. Pad sound with chorus
6. Lead sound with high resonance

## Technical Details

### Signal Flow
```
Input (MIDI Note) 
    → DCO (Sawtooth/Pulse/Sub) + Noise
    → VCF (Ladder Filter) 
    → VCA (Envelope)
    → HPF (High-pass Filter)
    → Soft Clip
    → Chorus
    → Stereo Output
```

### Modulation Routing
- LFO → DCO Pitch (±300 cents)
- LFO → VCF Cutoff (±6 octaves)
- LFO/Envelope → PWM
- Envelope → VCF Cutoff (±12 octaves)
- Keyboard → VCF Cutoff (tracking)
- Pitch Bend → DCO Pitch (±700 cents)
- Pitch Bend → VCF Cutoff (±4 octaves)

### Performance Optimizations
- Parameter updates once per audio buffer
- PolyBLEP for efficient anti-aliasing
- Exponential envelope curves for smooth transitions
- Parameter smoothing with single-pole filters

## Cross-Platform Compatibility

- M_PI defined for Windows compatibility
- Thread-safe random number generation
- Standard C++14 features only
- CMake build system for portability

## Future Enhancements

Potential improvements:
- Additional waveforms (sine, triangle)
- Filter key tracking amount control
- LFO rate modulation
- Velocity sensitivity controls
- MIDI CC mapping
- Preset management
