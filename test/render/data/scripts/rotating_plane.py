import nap
import math

def update(entity, currentTime, deltaTime):
	transform = entity.findComponent("nap::TransformComponentInstance")

	rot_speed = 0.1
	rot_angle = currentTime * 360.0 * rot_speed
	rot_angle_radians = math.radians(rot_angle)
	rotation = nap.rotate(nap.quat(), rot_angle_radians, nap.vec3(0.0, 1.0, 0.0))
	transform.setRotate(rotation)
