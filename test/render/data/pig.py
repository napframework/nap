import nap
import math

currentTime = 0

def update(elapsedTime):
	global currentTime
	currentTime += elapsedTime
	resourceMgr = core.getOrCreateService("nap::ResourceManagerService")
	worldEntity = resourceMgr.findEntity("WorldEntity")
	transform = worldEntity.findComponent("nap::TransformComponentInstance")
	transform.setTranslate(math.sin(currentTime*2) * 5.0, 0.0, -3.0)