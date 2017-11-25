import nap

nextNoteTime = 0

seq = [1, 4/5, 6/5, 0.5, 2/3, 4/3, 3/2]
index = 0

def update(entity, currentTime, deltaTime):
	global nextNoteTime
	global index
	if (currentTime >= nextNoteTime):
		nextNoteTime = currentTime + 0.1
		makeNote(entity, index, len(seq), 2/3)
		makeNote(entity, index + 2, len(seq) - 1, 6/5)
		index += 1


def makeNote(entity, index, len, fac):
	component = entity.findComponent("nap::audio::AudioComponentInstance")
	instrument = component.getObject()
	voice = instrument.findFreeVoice()
	oscA = voice.getObject("oscillatorA")
	oscB = voice.getObject("oscillatorB")
	oscA.getChannel(0).setFrequency(330 * seq[index % len] * fac, 0);
	oscA.getChannel(1).setFrequency(330 * 1.01 * seq[index % len] * fac, 0);
	oscB.getChannel(0).setFrequency(660 * seq[index % len] * fac, 0);
	oscB.getChannel(1).setFrequency(660 * 1.01 * seq[index % len] * fac, 0);
	instrument.play(voice, 0)
