(
s.waitForBoot({

	// SynthDef
	SynthDef(\junoArp, { |gate = 1, note = 60, amp = 0.8|  // note default set to middle C
		var sig;

		sig = Juno60.ar(
			gate: gate,
			note: note+60-12,
			amp: amp,
			attack: 0.01,
			decay: 0.1,
			sustain: 0.6,
			release: 0.8,
			cutoff: MouseY.kr(0.0, 1.0),  // Control cutoff with mouse Y position
			resonance: 0.3,
			envMod: MouseX.kr(0.0, 1.0),  // Control envelope modulation with mouse X position
			saw: 1,
			pulse: 0.3,
			sub: 0.8,
			noise: 0.0,
			pwm: 0.7,
			chorus: 4,
			chorusDrywet:  1.0,  // Control dry/wet with mouse X position
			lfoRate: 0.3,
			lfoDelay: 0.2,
			lfoAuto: 0,
			dcoLfo: 0.05,
			dcoPwmMod: 1,
			dcoRange: MouseY.kr(0.5, 2.0),  // Control oscillator range with mouse Y position
			hpf: 0.0,
			vcfLfo: 0.0,
			vcfKey: 0.8,
			vcfDir: 1.0,
			vcaType: 0.0,
			vcaValue: 0.0,
			doneAction: 2
		);

		Out.ar(0, sig);   // Ensure stereo output
	}).add;

	s.sync;

	// Directly play Pbind (no Routine needed)
	Pbind(
		\instrument, \junoArp,
		\dur, 0.125,
		\legato, 0.6,
		\amp, 0.8,
		\scale, Scale.major,
		\degree, Pseq([
			Pseq([0, 4, 7], 4),  // C
			Pseq([4, 9, 0], 4),  // Am/E
			Pseq([4, 7, 11-12], 4), // Em
			Pseq([7, 11-12, 14-12], 4) // G
		], inf)
	).play;
});
)
