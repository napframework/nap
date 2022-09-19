/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "focuscontroller.h"

// External includes
#include <transformcomponent.h>
#include <entity.h>
#include <mathutils.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

RTTI_BEGIN_CLASS(nap::FocusController)
	RTTI_PROPERTY("LookAtTarget",	&nap::FocusController::mLookAtTargetComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FocusControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// FocusController
	//////////////////////////////////////////////////////////////////////////

	void FocusController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// FocusControllerInstance
	//////////////////////////////////////////////////////////////////////////

	FocusControllerInstance::FocusControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool FocusControllerInstance::init(utility::ErrorState& errorState)
	{
		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;
		
		mResource = getComponent<FocusController>();
		return true;
	}


	void FocusControllerInstance::update(double deltaTime)
	{
		const auto focal = getFocalPosition();

		const auto eye = math::extractPosition(mTransformComponent->getGlobalTransform());
		const glm::vec3 dir = glm::normalize(getFocalPosition() - eye);

		mTransformComponent->setRotate(glm::rotation({ 0.0f, 0.0f, -1.0f }, dir));
	}


	glm::vec3 FocusControllerInstance::getFocalPosition() const
	{
		return math::extractPosition((*mLookAtTargetComponent).getGlobalTransform());
	}
}
