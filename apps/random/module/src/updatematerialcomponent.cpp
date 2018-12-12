#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
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

	}


	float* UpdateMaterialComponentInstance::getStaticWarmthPtr()
	{
		return &mStaticMeshComponent->getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uWarmth").mValue;
	}
}
