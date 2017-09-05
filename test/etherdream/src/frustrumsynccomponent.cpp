#include "frustrumsynccomponent.h"
#include <nap/entity.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	RTTI_DEFINE(nap::FrustrumSyncComponent)

	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FrustrumSyncComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&)
	RTTI_END_CLASS

	//////////////////////////////////////////////////////////////////////////

	bool FrustrumSyncComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Make sure we have a transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>(ETypeCheck::IS_DERIVED_FROM);
		if (!errorState.check(mTransform != nullptr, "missing transform component"))
			return false;

		// Get the output
		mOutput = getEntityInstance()->getParent()->findComponent<nap::LaserOutputComponentInstance>();
		if (!errorState.check(mOutput != nullptr, "no laser output component attached to parent"))
			return false;

		return true;
	}


	void FrustrumSyncComponentInstance::update(double deltaTime)
	{
		glm::vec2 laser_frustrum = mOutput->mProperties.mFrustrum;
		mTransform->setScale(glm::vec3(laser_frustrum.x, laser_frustrum.y, 1.0));
	}


	void FrustrumSyncComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components)
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}
}