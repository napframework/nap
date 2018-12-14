#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("CameraTransformComponent", &nap::UpdateMaterialComponent::mCameraTransformComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunCloudsMeshComponent", &nap::UpdateMaterialComponent::mSunCloudsMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunGlareMeshComponent", &nap::UpdateMaterialComponent::mSunGlareMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StaticMeshComponent", &nap::UpdateMaterialComponent::mStaticMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitComponent", &nap::UpdateMaterialComponent::mOrbitComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LightRigEntity", &nap::UpdateMaterialComponent::mLightRigEntity, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UpdateMaterialComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		// Call an intial update to apply properties
		updateSunGlareOrbit();

		return true;
	}


	void UpdateMaterialComponentInstance::update(double deltaTime)
	{
		// Update clouds shader
		updateSunClouds(deltaTime);

		// Update camera location
		updateCameraLocation();
	}


	void UpdateMaterialComponentInstance::updateCameraLocation()
	{
		// Retrieve camera location
		glm::vec3 cam_pos = math::extractPosition(mCameraTransformComponent->getGlobalTransform());

		// Get all renderable meshes under the light rig and set camera location uniform (they should all have one)
		std::vector<RenderableMeshComponentInstance*> render_meshes;
		mLightRigEntity->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(render_meshes);
		for (auto& rmesh : render_meshes)
		{
			nap::MaterialInstance& material = rmesh->getMaterialInstance();
			material.getOrCreateUniform<nap::UniformVec3>("cameraLocation").setValue(cam_pos);
		}
	}


	void UpdateMaterialComponentInstance::updateSunClouds(double deltaTime)
	{
		// Set inverted property on shader in case the mSunCloudsInverted boolean changed
		mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInverted").mValue = mSunCloudsInverted ? 1.0f : 0.0f;
		// Apply wind and noise forces to cloud shader
		glm::vec3* sunCloudsOffset = &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOffset").mValue;
		float sunCloudsRotation = mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
		float windDirectionRad = nap::math::radians(sunCloudsRotation);
		float windDistance = mSunCloudsWindSpeed * (float)deltaTime;
		sunCloudsOffset->x += cos(windDirectionRad) * windDistance;
		sunCloudsOffset->y += sin(windDirectionRad) * windDistance;
		sunCloudsOffset->z += mSunCloudsNoiseSpeed * (float)deltaTime;
	}


	void UpdateMaterialComponentInstance::updateSunGlareOrbit()
	{
		mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitAngle").mValue = mOrbitComponent->getAngle();
		mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOrbitRadius").mValue = mOrbitComponent->mRadius;
		glm::vec3* sunGlareOrbitCenter = &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("uOrbitCenter").mValue;
		sunGlareOrbitCenter->x = mOrbitComponent->mCenter[0];
		sunGlareOrbitCenter->y = mOrbitComponent->mCenter[1];
	}


	float* UpdateMaterialComponentInstance::getSunCloudsRotationPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uRotation").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsContrastPtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uContrast").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunCloudsScalePtr()
	{
		return &mSunCloudsMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uScale").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareOuterSizePtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uOuterSize").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareInnerSizePtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uInnerSize").mValue;
	}


	float* UpdateMaterialComponentInstance::getSunGlareStretchPtr()
	{
		return &mSunGlareMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uStretch").mValue;
	}


	float* UpdateMaterialComponentInstance::getStaticWarmthPtr()
	{
		return &mStaticMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWarmth").mValue;
	}
}
