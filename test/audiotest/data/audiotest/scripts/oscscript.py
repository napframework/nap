import nap

def handleOSC(event):
	print("OSC input")

def init(entity):
	print("init")
	oscInput = entity.findComponent("nap::OSCInputComponentInstance")
	oscInput.getMessageReceived().connect(handleOSC)

def update(entity, currentTime, deltaTime):
    pass


