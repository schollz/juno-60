(
s.waitForBoot({
	SynthDef("juno",{
		Out.ar(0,Juno60.ar(
			gate: 1,
			note:  60, // actually note
			amp: 0.5,
			attack: 2,
			decay: 0.4,
			sustain: 0.4,
				release: 0.2,
			cutoff: 0.8,
			resonance: 0.3,
			envMod: 0.4,
			saw: 1,
			pulse: 1.0,
			sub: 0.2,
			noise: 0.1,
			pwm: 0.0,
			chorus: 1,
			doneAction: 2
		).poll);
	}).add;
	
	s.sync;
	
		Routine {
			["gate 1"].postln;
			a = Synth("juno");
			0.25.wait;
			["note change"].postln;
			a.set(\note, 67);
		2.wait;
		["gate 0"].postln;
		a.set(\gate,0);
		10.wait;
		s.quit;
			0.2.wait;
			0.exit;
		}.play(SystemClock);

});
)
