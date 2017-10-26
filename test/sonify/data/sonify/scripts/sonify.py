import nap

nextNoteTime = 0

index = 0

def update(entity, currentTime, deltaTime):
	global nextNoteTime
	global index
	if (currentTime >= nextNoteTime):
		nextNoteTime = currentTime + 0.1
		freq = 220
		amp = 0.1
		dur = 0.1
		makeNote(entity, index, freq, amp, dur)
		index += 1


def makeNote(entity, index, freq, amp, dur):
	component = entity.findComponent("nap::audio::AudioComponentInstance")
	instrument = component.getObject()
	voice = instrument.findFreeVoice()
	osc = voice.getObject("oscillator")
	osc.getChannel(0).setFrequency(freq);
	gain = voice.getObject("gain")
	instrument.play(voice, dur)
	gain.getChannel(0).setGain(amp)
