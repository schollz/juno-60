(
s.waitForBoot({
	SynthDef("juno",{
		Out.ar(0,Juno60.ar(
        gate: 1,
		freq: 60, // actually note
        amp: 0.5,
        attack: 2,
        decay: 0.4,
        sustain: 0.4,
        release: 0.6,
        cutoff: 0.6,
        resonance: 0.3,
        envMod: 0.4,
        saw: 1,
		cutoff: 0.8,
        pulse: 1.0,
        sub: 0.2,
        noise: 0.1,
        pwm: 0.0,
        chorus: 1
		).poll);
	}).add;
	
	s.sync;
	
	Routine {
		["gate 1"].postln;
		a = Synth("juno");
		1.wait;
		a.set(\gate,1);
		4.wait;
		["gate 0"].postln;
		a.set(\gate,0);
	}.play;

});
)
