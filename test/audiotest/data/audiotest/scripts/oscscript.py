import nap

def handleOSC(event):
	print("osc python")
	print(event.getAddress())

def init(entity):
	print("init")
	oscInput = entity.findComponent("nap::OSCInputComponentInstance")
	oscInput.getMessageReceived().connect(handleOSC)

def update(entity, currentTime, deltaTime):
    pass


