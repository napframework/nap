import nap
import math

def update(entity, currentTime, deltaTime):
	transform = entity.findComponent("nap::TransformComponentInstance")

	translate = transform.getTranslate()
	translate = nap.vec3(math.sin(currentTime*2), 0.0, -3.0)
	transform.setTranslate(translate)

