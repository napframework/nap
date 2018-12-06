#include "animationcomponent.h"

#include <entity.h>
#include <transformcomponent.h>
#include <rect.h>
#include <glm/gtx/transform.hpp>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::AnimatorComponent)
	RTTI_PROPERTY("Curve", &nap::AnimatorComponent::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AnimatorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&) 
RTTI_END_CLASS

namespace nap
{
	AnimatorComponentInstance::AnimatorComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{ }


	bool AnimatorComponentInstance::init(utility::ErrorState& errorState)
	{
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		return true;
	}


	void AnimatorComponentInstance::update(double deltaTime)
	{
		auto animator = getComponent<AnimatorComponent>();
		float y = animator->mCurve.get()->evaluate(mLocalTime);
		glm::vec3 pos(0, y, 0);
		mTransform->setTranslate(pos);

		mLocalTime = fmod(mLocalTime + deltaTime, 1);
	}
}