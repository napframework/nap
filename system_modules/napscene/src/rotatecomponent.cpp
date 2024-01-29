/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rotatecomponent.h"
#include "entity.h"

//////////////////////////////////////////////////////////////////////////
// RTTI
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::RotateProperties)
	RTTI_PROPERTY("Axis",	&nap::RotateProperties::mAxis,		nap::rtti::EPropertyMetaData::Default, "Rotation axis (x, y, z)")
	RTTI_PROPERTY("Speed",	&nap::RotateProperties::mSpeed,		nap::rtti::EPropertyMetaData::Default, "Rotation speed in seconds, where 1 second = 360*")
	RTTI_PROPERTY("Offset",	&nap::RotateProperties::mOffset,	nap::rtti::EPropertyMetaData::Default, "Rotation offset in seconds, where 1 second = 360*")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RotateComponent, "Rotates an entity along the given axis at the designated speed")
	RTTI_PROPERTY("Properties", &nap::RotateComponent::mProperties, nap::rtti::EPropertyMetaData::Required, "Rotation settings")
	RTTI_PROPERTY("Enabled", &nap::RotateComponent::mEnabled, nap::rtti::EPropertyMetaData::Default, "If rotation is enabled")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RotateComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Rotate Component
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool RotateComponentInstance::init(utility::ErrorState& errorState)
	{
		// Make sure we have a transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;
		
		// Copy over properties
		mProperties = getComponent<RotateComponent>()->mProperties;
		mEnabled = getComponent<RotateComponent>()->mEnabled;

		// Copy initial rotation
		mInitialRotate = mTransform->getRotate();

		return true;
	}


	void RotateComponentInstance::update(double deltaTime)
	{
		if (mEnabled)
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


	void RotateComponentInstance::reset()
	{
		mElapsedTime = 0.0f;
	}
}
