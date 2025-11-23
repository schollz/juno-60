// Juno60 Test File
// This file demonstrates the Juno60 UGen

// Boot the server
s.boot;

(
// Test 1: Basic sawtooth sound
{
    var gate, freq, sig;
    gate = Impulse.kr(2);  // Trigger notes
    freq = 60;  // MIDI note 60 (middle C)
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.5,
        attack: 0.01,
        decay: 0.3,
        sustain: 0.5,
        release: 0.5,
        cutoff: 0.7,
        resonance: 0.2,
        envMod: 0.5,
        saw: 1,
        pulse: 0,
        sub: 0,
        noise: 0,
        pwm: 0.5,
        chorus: 0
    );
    sig * 0.5;
}.play;
)

(
// Test 2: Pulse wave with PWM
{
    var gate, freq, sig;
    gate = Impulse.kr(1);
    freq = 48;  // Lower note
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.6,
        attack: 0.02,
        decay: 0.5,
        sustain: 0.3,
        release: 0.8,
        cutoff: 0.5,
        resonance: 0.4,
        envMod: 0.7,
        saw: 0,
        pulse: 1,
        sub: 0,
        noise: 0,
        pwm: 0.7,
        chorus: 0
    );
    sig * 0.5;
}.play;
)

(
// Test 3: Mixed waveforms with chorus
{
    var gate, freq, sig;
    gate = Impulse.kr(1.5);
    freq = Seq([60, 64, 67, 72]).kr(gate);  // Arpeggio
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.5,
        attack: 0.05,
        decay: 0.4,
        sustain: 0.4,
        release: 0.6,
        cutoff: 0.6,
        resonance: 0.3,
        envMod: 0.4,
        saw: 1,
        pulse: 1,
        sub: 0.3,
        noise: 0.1,
        pwm: 0.5,
        chorus: 1  // Chorus mode I
    );
    sig * 0.4;
}.play;
)

(
// Test 4: Bass sound with sub oscillator
{
    var gate, freq, sig;
    gate = Impulse.kr(2);
    freq = 36;  // Low C
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.7,
        attack: 0.001,
        decay: 0.3,
        sustain: 0.2,
        release: 0.4,
        cutoff: 0.3,
        resonance: 0.5,
        envMod: 0.8,
        saw: 1,
        pulse: 0,
        sub: 0.8,
        noise: 0,
        pwm: 0.5,
        chorus: 0
    );
    sig * 0.5;
}.play;
)

(
// Test 5: Pad sound with chorus
{
    var gate, freq, sig;
    gate = Impulse.kr(0.5);
    freq = Seq([60, 64, 67, 64]).kr(gate);
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.4,
        attack: 0.5,
        decay: 0.3,
        sustain: 0.7,
        release: 1.5,
        cutoff: 0.4,
        resonance: 0.1,
        envMod: 0.2,
        saw: 1,
        pulse: 1,
        sub: 0,
        noise: 0.05,
        pwm: 0.6,
        chorus: 3  // Chorus mode I+II
    );
    sig * 0.6;
}.play;
)

(
// Test 6: Lead sound with high resonance
{
    var gate, freq, sig;
    gate = Impulse.kr(4);
    freq = Seq([72, 74, 76, 77, 76, 74]).kr(gate);
    sig = Juno60.ar(
        gate: gate,
        freq: freq,
        amp: 0.6,
        attack: 0.01,
        decay: 0.2,
        sustain: 0.5,
        release: 0.3,
        cutoff: 0.6,
        resonance: 0.7,
        envMod: 0.6,
        saw: 1,
        pulse: 0,
        sub: 0,
        noise: 0,
        pwm: 0.5,
        chorus: 2  // Chorus mode II
    );
    sig * 0.4;
}.play;
)

// Stop all sounds
s.freeAll;
