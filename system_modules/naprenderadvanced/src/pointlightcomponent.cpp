/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pointlightcomponent.h"

// Local includes
#include "renderadvancedservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <transformcomponent.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// nap::PointLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::PointLightComponent, "Omnidirectional light that emits from its origin")
	RTTI_PROPERTY("Attenuation",	&nap::PointLightComponent::mAttenuation,	nap::rtti::EPropertyMetaData::Default, "Light intensity decay rate, higher values = less light further away from the source")
	RTTI_PROPERTY("ClippingPlanes",	&nap::PointLightComponent::mClippingPlanes,	nap::rtti::EPropertyMetaData::Default, "Shadow camera near and far clipping planes")
	RTTI_PROPERTY("ShadowMapSize",	&nap::PointLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default, "Resolution of the shadow texture")
RTTI_END_CLASS

// nap::PointLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_PROPERTY(nap::uniform::light::attenuation, &nap::PointLightComponentInstance::mAttenuation, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool PointLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		// Copy resources
		auto* resource = getComponent<PointLightComponent>();
		mAttenuation = resource->mAttenuation;
		mShadowMapSize = resource->mShadowMapSize;

		// Register light properties
		registerUniformLightProperty(uniform::light::attenuation);

		// Create shadow camera resource
		std::string uuid = math::generateUUID();
		mShadowCamEntity = std::make_unique<Entity>();
		mShadowCamEntity->mID = utility::stringFormat("%s_shadow_%s", getEntityInstance()->mID.c_str(), uuid.c_str());

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("%s_shadow_xform_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowCamXformComponent.get());

		// Perspective camera component
		mShadowCamComponent = std::make_unique<PerspCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("%s_shadow_camera_%s",getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mFieldOfView = 90.0f;
		mShadowCamEntity->mComponents.emplace_back(mShadowCamComponent.get());

		// Shadow Origin component
		mGnomonMesh = std::make_unique<GnomonMesh>(*getEntityInstance()->getCore());
		mGnomonMesh->mID = utility::stringFormat("%s_shadow_gnomon_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mGnomonMesh->mSize = mResource->mLocator.mGnomonSize;
		if (!mGnomonMesh->init(errorState))
			return false;

		mShadowOriginComponent = std::make_unique<RenderGnomonComponent>();
		mShadowOriginComponent->mID = utility::stringFormat("%s_shadow_origin_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowOriginComponent->mDepthMode = EDepthMode::ReadOnly;
		mShadowOriginComponent->mMesh = mGnomonMesh.get();
		mShadowOriginComponent->mLineWidth = mResource->mLocator.mLineWidth;
		mShadowCamEntity->mComponents.emplace_back(mShadowOriginComponent.get());

		// Spawn it
		if (spawnShadowCamera(*mShadowCamEntity, errorState) == nullptr)
		{
			errorState.fail("Unable to spawn directional spotlight shadow entity");
			return false;
		}
		return true;
	}
}
