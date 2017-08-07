import nap
import math

def update(entity, currentTime, deltaTime):
	transform = entity.findComponent("nap::TransformComponentInstance")

	resource_manager = nap.core.getOrCreateService("nap::ResourceManagerService")
	test_texture = resource_manager.findObject("TestTexture")

	material_instance = entity.findComponent("nap::RenderableMeshComponentInstance").getMaterialInstance()
	material_instance.getOrCreateUniform("testTexture").setTexture(test_texture)
	material_instance.getOrCreateUniform("pigTexture").setTexture(test_texture)
	material_instance.getOrCreateUniform("mTextureIndex").setValue(0)
	material_instance.getOrCreateUniform("mColor").setValue(nap.vec4(1,1,1,1))
