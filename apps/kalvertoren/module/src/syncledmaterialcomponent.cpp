#include "syncledmaterialcomponent.h"

// External Includes
#include <entity.h>
#include <material.h>

// nap::updatematerialcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SyncLedMaterialComponent)
	RTTI_PROPERTY("Pointlight", &nap::SyncLedMaterialComponent::mPointLightComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointlightTransform", &nap::SyncLedMaterialComponent::mPointLightTransform, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraTransform", &nap::SyncLedMaterialComponent::mCameraTransform, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SelectMeshComponent", &nap::SyncLedMaterialComponent::mSelectMeshComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::updatematerialcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SyncLedMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SyncLedMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool SyncLedMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void SyncLedMaterialComponentInstance::update(double deltaTime)
	{
		// Set cam location
		const glm::mat4x4& cam_xform = mCameraTransform->getGlobalTransform();
		glm::vec3 cam_pos(cam_xform[3][0], cam_xform[3][1], cam_xform[3][2]);

		// Set light location
		const glm::mat4x4 light_xform = mPointLightTransform->getGlobalTransform();
		glm::vec3 light_pos(light_xform[3][0], light_xform[3][1], light_xform[3][2]);

		// Set light intensity
		glm::vec3 intensity = { mPointlightComponent->mIntensity, mPointlightComponent->mIntensity, mPointlightComponent->mIntensity };

		for (auto& frame_mesh : mLedSelectComponent->getFrameMeshes())
		{
			nap::MaterialInstance& frame_material = frame_mesh.getMaterialInstance();
			
			nap::UniformVec3& frame_cam_pos = frame_material.getOrCreateUniform<nap::UniformVec3>("cameraLocation");
			frame_cam_pos.setValue(cam_pos);

			nap::UniformVec3& frame_light_pos = frame_material.getOrCreateUniform<nap::UniformVec3>("lightPosition");
			frame_light_pos.setValue(light_pos);

			nap::UniformVec3& frame_light_intensity = frame_material.getOrCreateUniform<nap::UniformVec3>("lightIntensity");
			frame_light_intensity.setValue(intensity);
		}

		for (auto& triangle_mesh : mLedSelectComponent->getTriangleMeshes())
		{
			nap::MaterialInstance& triangle_material = triangle_mesh.getMaterialInstance();

			nap::UniformVec3& triangle_cam_pos = triangle_material.getOrCreateUniform<nap::UniformVec3>("cameraLocation");
			triangle_cam_pos.setValue(cam_pos);

			nap::UniformVec3& triangle_light_pos = triangle_material.getOrCreateUniform<nap::UniformVec3>("lightPosition");
			triangle_light_pos.setValue(light_pos);

			nap::UniformVec3& triangle_light_intensity = triangle_material.getOrCreateUniform<nap::UniformVec3>("lightIntensity");
			triangle_light_intensity.setValue(intensity);
		}
	}
}
