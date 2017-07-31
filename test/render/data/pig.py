import nap
import core
import math

currentTime = 0

def update(elapsedTime):
	global currentTime
	currentTime += elapsedTime
	resourceMgr = core.resourceManagerService
	worldEntity = resourceMgr.findEntity("WorldEntity")
	transform = worldEntity.findComponentPython("nap::TransformComponentInstance")
	transform.setTranslate(math.sin(currentTime*2) * 5.0, 0.0, -3.0)