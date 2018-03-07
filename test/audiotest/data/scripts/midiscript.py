import nap
import math

global audioComponent

def handleNoteOn(event):	
	global audioComponent
	if event.getVelocity() == 0:
		return

	graph = audioComponent.getObject()
	oscA = graph.getObject("oscA")
	oscB = graph.getObject("oscB")
	f = mtof(event.getNoteNumber())
	oscA.getChannel(0).setFrequency(f, 10)
	oscB.getChannel(0).setFrequency(f, 10)
	print(event.getNoteNumber())
	print(f)

def handleNoteOff(event):
	global audioComponent
	graph = audioComponent.getObject()
	oscB = graph.getObject("oscB")
	oscB.getChannel(0).setAmplitude(0, 10)

def handleCC(event): 
	global audioComponent
	graph = audioComponent.getObject()
	oscA = graph.getObject("oscA")
	oscB = graph.getObject("oscB")
	a = mtoa(event.getVelocity())
	oscA.getChannel(0).setAmplitude(a * 0.25, 10)
	oscB.getChannel(0).setAmplitude(a * 0.25, 10)

def init(e):
	global audioComponent
	print("init")
	e.findComponentByID("controlChange").getMessageReceived().connect(handleCC)
	e.findComponentByID("noteOn").getMessageReceived().connect(handleNoteOn)
	e.findComponentByID("noteOff").getMessageReceived().connect(handleNoteOff)
	audioComponent = e.findComponentByID("audioComponent")

def update(e, currentTime, deltaTime):
    pass

def mtof(midiNote):
	return (440 / 32) * pow(2, (midiNote - 9) / 12)

def mtoa(velocity):
    if velocity == 0:
        a = 0
    else:
        dB = (-24 + (24 * (velocity / 100))) / 10
        a = pow(10, dB)
        
    return a;


