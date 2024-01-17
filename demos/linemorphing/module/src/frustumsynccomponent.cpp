/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "frustumsynccomponent.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	RTTI_BEGIN_CLASS(nap::FrustumSyncComponent)
		RTTI_PROPERTY("LaserOutputComponent", &nap::FrustumSyncComponent::mLaserOutputComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_END_CLASS

	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FrustumSyncComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_END_CLASS

	//////////////////////////////////////////////////////////////////////////

	bool FrustumSyncComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get the component that created this instance
		FrustumSyncComponent* resource = getComponent<FrustumSyncComponent>();

		// Make sure there's at least one child entity (the laser canvas)
		if (!errorState.check(getEntityInstance()->getChildren().size() == 1, "Expected one child"))
			return false;

		// Get the canvas entity
		EntityInstance* laser_draw_entity = getEntityInstance()->getChildren()[0];

		// Make sure that the visualizer has a transform
		mCanvasTransform = laser_draw_entity->findComponent<TransformComponentInstance>();
		if (!errorState.check(mCanvasTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Move the frustrum back a bit so objects around 0 are sorted correctly
		mCanvasTransform->setTranslate(glm::vec3(0.0f, 0.0f, -0.001f));

		return true;
	}


	void FrustumSyncComponentInstance::update(double deltaTime)
	{
		// Sync the canvas to the laser output dimensions
		glm::vec2 laser_frustrum = mOutput->mProperties.mFrustum;
		mCanvasTransform->setScale(glm::vec3(laser_frustrum.x, laser_frustrum.y, 1.0));
	}


	void FrustumSyncComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}
}