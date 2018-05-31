import nap

nextNoteTime = 0
nextVoiceCountTime = 0
instrument = None

seq = [1, 4/5, 6/5, 0.5, 2/3, 4/3, 3/2, 6/5, 2]
index = 0

def update(entity, currentTime, deltaTime):
	global nextNoteTime
	global nextVoiceCountTime
	global index
	global instrument
	if (currentTime >= nextNoteTime):
		nextNoteTime = currentTime + 0.1
		makeNote(entity, index, len(seq), 2/3)
		makeNote(entity, index + 2, len(seq) - 1, 6/5)
		index += 1
	if (currentTime >= nextVoiceCountTime):
		print(instrument.getBusyVoiceCount())
		nextVoiceCountTime += 1


def makeNote(entity, index, len, fac):
	global instrument
	voice = instrument.findFreeVoice()
	oscA = voice.getObject("oscillatorA")
	oscB = voice.getObject("oscillatorB")
	oscA.getChannel(0).setFrequency(660 * seq[index % len] * fac, 0);
	oscA.getChannel(1).setFrequency(660 * 1.01 * seq[index % len] * fac, 0);
	oscB.getChannel(0).setFrequency(440 * seq[index % len] * fac, 0);
	oscB.getChannel(1).setFrequency(440 * 1.01 * seq[index % len] * fac, 0);
	if (index % 3 != 0):
		oscB.getChannel(0).fmInput.disconnect(oscA.getChannel(0).audioOutput)
		oscB.getChannel(1).fmInput.disconnect(oscA.getChannel(1).audioOutput)
	else:
		oscB.getChannel(0).fmInput.connect(oscA.getChannel(0).audioOutput)
		oscB.getChannel(1).fmInput.connect(oscA.getChannel(1).audioOutput)

	instrument.play(voice, 0)

def init(entity):
	global instrument
	component = entity.findComponent("nap::audio::AudioComponentInstance")
	instrument = component.getObject()
	pass
