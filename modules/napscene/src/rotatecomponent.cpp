// Local Includes
#include "rotatecomponent.h"
#include <nap/entityinstance.h>

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::RotateProperties)
	RTTI_PROPERTY("Axis",	&nap::RotateProperties::mAxis,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",	&nap::RotateProperties::mSpeed,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset",	&nap::RotateProperties::mOffset,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RotateComponent)
	RTTI_PROPERTY("Properties", &nap::RotateComponent::mProperties, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::RotateComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Rotate Component
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool RotateComponentInstance::init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		assert(resource->get_type().is_derived_from<RotateComponent>());
		
		// Make sure we have a transform
		mTransform = getEntity()->findComponent<TransformComponent>(ETypeCheck::IS_DERIVED_FROM);
		if (!errorState.check(mTransform != nullptr, "missing transform component"))
			return false;
		
		// Copy over properties
		mProperties = rtti_cast<RotateComponent>(resource.get())->mProperties;

		// Copy initial rotation
		mInitialRotate = mTransform->getRotate();

		return true;
	}


	void RotateComponentInstance::update(double deltaTime)
	{
		// Update elapsed time taking in to account rotation speed
		mElapsedTime += (deltaTime * mProperties.mSpeed);
		
		// Calculate rotation angle including offset
		float rot_angle = (mElapsedTime + mProperties.mOffset) * 360.0f;
		glm::quat new_ror = glm::rotate(mInitialRotate, glm::radians(rot_angle), mProperties.mAxis);

		// Set new rotation
		mTransform->setRotate(new_ror);
	}

}