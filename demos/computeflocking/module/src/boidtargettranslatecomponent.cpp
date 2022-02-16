/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "boidtargettranslatecomponent.h"
#include "entity.h"

// External Includes
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::BoidTargetTranslateComponent)
	RTTI_PROPERTY("Radius", &nap::BoidTargetTranslateComponent::mRadius, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed", &nap::BoidTargetTranslateComponent::mSpeed, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BoidTargetTranslateComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	bool BoidTargetTranslateComponentInstance::init(utility::ErrorState& errorState)
	{
		// Make sure we have a transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		auto* resource = getComponent<BoidTargetTranslateComponent>();
		mRadius = resource->mRadius;
		mSpeed = resource->mSpeed;

		return true;
	}


	void BoidTargetTranslateComponentInstance::update(double deltaTime)
	{
		// Update elapsed time taking in to account rotation speed
		mElapsedTime += (deltaTime * mSpeed);

		// Calculate new target position
		// Use simple non-periodic function to generate a cheap to compute noise signal
		glm::quat orientation = { {
			static_cast<float>(glm::sin(2.0 * mElapsedTime) + glm::sin(math::PI * mElapsedTime)) * 0.5f,
			static_cast<float>(glm::sin(4.0 * mElapsedTime) + glm::sin(math::PI * mElapsedTime)) * 0.5f,
			static_cast<float>(glm::sin(0.5 * mElapsedTime) + glm::sin(math::PI * mElapsedTime)) * 0.5f
		} };

		float magnitude = static_cast<float>(glm::sin(0.25 * mElapsedTime) + glm::sin(math::PI * mElapsedTime)) * 0.5f * mRadius;

		// Set new translation
		mTransform->setTranslate((orientation * mForward) * magnitude);
	}


	void BoidTargetTranslateComponentInstance::reset()
	{
		mElapsedTime = 0.0f;
	}
}
