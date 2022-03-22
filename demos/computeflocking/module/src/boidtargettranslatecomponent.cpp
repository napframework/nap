/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "boidtargettranslatecomponent.h"
#include "entity.h"

// External Includes
#include <mathutils.h>
#include <glm/gtc/noise.hpp>

RTTI_BEGIN_CLASS(nap::BoidTargetTranslateComponent)
	RTTI_PROPERTY("Radius", &nap::BoidTargetTranslateComponent::mRadius, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed", &nap::BoidTargetTranslateComponent::mSpeed, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomOffset", &nap::BoidTargetTranslateComponent::mRandomOffset, nap::rtti::EPropertyMetaData::Default)
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
		mSpeed = glm::max(resource->mSpeed, 0.0f);

		if (resource->mRandomOffset)
		{
			mOffset = math::random<float>(-9999.0f, 9999.0f);
		}
		return true;
	}


	void BoidTargetTranslateComponentInstance::update(double deltaTime)
	{
		// Update elapsed time taking in to account rotation speed
		mElapsedTime += static_cast<float>(deltaTime) * mSpeed;

		// Calculate new target position
		// Use simple non-periodic function to generate a cheap to compute noise signal
		float offset = mElapsedTime + mOffset;

		const glm::vec2 n0 = { offset, 1.2344980816f };
		const glm::vec2 n1 = { offset, 2.7702631534f };
		const glm::vec2 n2 = { offset, 3.5157356376f };
		const glm::vec2 n3 = { offset, 0.8180252648f };

		glm::quat orientation = { {
			glm::simplex(n0),
			glm::simplex(n1),
			glm::simplex(n2)
		} };

		float magnitude = math::lerp(0.1f, 1.0f, glm::simplex(n3)) * mRadius;

		// Set new translation
		mTransform->setTranslate((orientation * mForward) * magnitude);
	}


	void BoidTargetTranslateComponentInstance::reset()
	{
		mElapsedTime = 0.0f;
	}


	void BoidTargetTranslateComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}
}
