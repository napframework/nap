#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("SunCloudsMeshComponent", &nap::UpdateMaterialComponent::mSunCloudsMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SunGlareMeshComponent", &nap::UpdateMaterialComponent::mSunGlareMeshComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StaticMeshComponent", &nap::UpdateMaterialComponent::mStaticMeshComponent, nap::rtti::EPropertyMetaData::Required)
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
		return true;
	}


	void UpdateMaterialComponentInstance::update(double deltaTime)
	{

		// Update clouds shader
		updateClouds(deltaTime);
	}


	void UpdateMaterialComponentInstance::updateClouds(double deltaTime)
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


	float* UpdateMaterialComponentInstance::getStaticWarmthPtr()
	{
		return &mStaticMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWarmth").mValue;
	}
}
