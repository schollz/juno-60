(
s.waitForBoot({
	var a;
		SynthDef("juno",{ |gate = 1, note = 48|
			Out.ar(0,Juno60.ar(
				gate: gate,
				note: note,
				amp: 1.0,
				attack: 2.0,
				decay: 0.0,
				sustain: 1.0,
				release: 2.0,
				cutoff: 0.3,
				resonance: 0.2,
				envMod: 0.8,
				saw: 1,
				pulse: 0.3,
				sub: 1.0,
				noise: 0.0,
				pwm: 0.7,
				chorus: 4,
				chorusDrywet: 1.0,
				lfoRate: 0.3,
				lfoDelay: 0.8,
				lfoAuto: 0,
				dcoLfo: 0.05,
				dcoPwmMod: 1,
				dcoRange: 1.0,
				hpf: 0.0,
				vcfLfo: 0.0,
				vcfKey: 0.8,
				vcfDir: 1.0,
				vcaType: 0.0,
				vcaValue: 0.0,
				doneAction: 2
			));
		}).add;
	
	s.sync;
	
	["gate 1"].postln;
	a = Synth("juno", [\gate, 1, \note, 60-24]);
	b = Synth("juno", [\gate, 1, \note, 64-12]);
	c = Synth("juno", [\gate, 1, \note, 67]);

	[2.0,3.0,4.0,5.0,6.0].do { |t|
		SystemClock.sched(t, {
			// print out CPU usage
			["CPU Usage: ", Server.default.avgCPU, Server.default.peakCPU].postln;
			nil;
		});
	};
	
	SystemClock.sched(2.0, {
		["gate 0"].postln;
		a.set(\gate, 0);
		b.set(\gate, 0);
		c.set(\gate, 0);
		nil;
	});
	
	SystemClock.sched(4.1, {
		0.exit;
		nil;
	});

});
)
