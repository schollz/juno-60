(
s.waitForBoot({
	var a;
		SynthDef("juno",{ |gate = 1, note = 48|
			Out.ar(0,Juno60.ar(
				gate: gate,
				note: note,
				amp: 1.0,
				attack: 0.3,
				decay: 0.0,
				sustain: 1.0,
				release: 0.6,
				cutoff: 0.5,
				resonance: 0.0,
				envMod: 0.0,
				saw: 1,
				pulse: 1.0,
				sub: 1.0,
				noise: 0.0,
				pwm: 0.7,
				chorus: 2,
				lfoRate: 0.3,
				lfoDelay: 0.8,
				lfoAuto: 0,
				dcoLfo: 0.0,
				dcoPwmMod: 1,
				dcoRange: 1.0,
				hpf: 0.0,
				vcfLfo: 0.0,
				vcfKey: 1.0,
				vcfDir: 1.0,
				vcaType: 0.0,
				vcaValue: 0.3,
				doneAction: 2
			).poll);
		}).add;
	
	s.sync;
	
	["gate 1"].postln;
	a = Synth("juno");
	
	SystemClock.sched(0.25, {
		["note change"].postln;
		a.set(\note, 65);
		nil;
	});
SystemClock.sched(4.65, {
	["gate 0"].postln;
	a.set(\gate, 0);
	nil;
	});
	
	SystemClock.sched(10.5, {
		0.exit;
		nil;
	});

});
)
