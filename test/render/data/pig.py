import nap
import math

currentTime = 0

def update(elapsedTime):
	global currentTime
	currentTime += elapsedTime
	resourceMgr = nap.core.getOrCreateService("nap::ResourceManagerService")
	worldEntity = resourceMgr.findEntity("WorldEntity")
	transform = worldEntity.findComponent("nap::TransformComponentInstance")
	
	translate = transform.getTranslate()
	translate = nap.vec3(math.sin(currentTime*2), 0.0, 0.0)
	translate.z = -3.0
	transform.setTranslate(translate)