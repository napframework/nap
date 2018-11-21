#include "renderablecopymeshcomponent.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>

// nap::renderablecopymeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableCopyMeshComponent)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableCopyMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorUniform",		&nap::RenderableCopyMeshComponent::mColorUniform,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::renderablecopymeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableCopyMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderableCopyMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool RenderableCopyMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableCopyMeshComponent* resource = getComponent<RenderableCopyMeshComponent>();

		// Fetch transform, used to offset the copied meshes
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if(!errorState.check(mTransform != nullptr, "%s: unable to find transform component", resource->mID.c_str()))
			return false;

		// Initialize our material instance based on values in the resource
		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
			return false;

		return true;
	}


	void RenderableCopyMeshComponentInstance::update(double deltaTime)
	{

	}


	void RenderableCopyMeshComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{

	}

}