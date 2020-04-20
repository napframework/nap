import nap
import math

class WorldEntity:
	def __init__(self, entity):
		self.entity = entity

	def update(self, currentTime, deltaTime):
		transform = self.entity.findComponent("nap::TransformComponentInstance")

		translate = transform.getTranslate()
		translate = nap.vec3(math.sin(currentTime*2), 0.0, -3.0)
		transform.setTranslate(translate)

	def destroy(self):
		pass