/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "spotlightcomponent.h"
#include "renderadvancedservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <transformcomponent.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>

// nap::SpotLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SpotLightComponent)
	RTTI_PROPERTY("Attenuation",			&nap::SpotLightComponent::mAttenuation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Angle",					&nap::SpotLightComponent::mAngle,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FieldOfViewClip",		&nap::SpotLightComponent::mFOVClip,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Falloff",				&nap::SpotLightComponent::mFalloff,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClippingPlanes",			&nap::SpotLightComponent::mClippingPlanes,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize",			&nap::SpotLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::SpotLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SpotLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_PROPERTY(nap::uniform::light::attenuation, &nap::SpotLightComponentInstance::mAttenuation, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY(nap::uniform::light::angle,		&nap::SpotLightComponentInstance::mAngle,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY(nap::uniform::light::falloff,		&nap::SpotLightComponentInstance::mFalloff,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SpotLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		// Copy resource properties
		auto* resource = getComponent<SpotLightComponent>();
		mAttenuation = resource->mAttenuation;
		mFalloff = resource->mFalloff;
		mShadowMapSize = resource->mShadowMapSize;
		mFOVClip = resource->mFOVClip;

		// Register uniforms to auto push
		registerUniformLightProperty(uniform::light::attenuation);
		registerUniformLightProperty(uniform::light::angle);
		registerUniformLightProperty(uniform::light::falloff);
	
		// Create shadow camera resource
		std::string uuid = math::generateUUID();
		mShadowCamEntity = std::make_unique<Entity>();
		mShadowCamEntity->mID = utility::stringFormat("%s_shadow_%s", getEntityInstance()->mID.c_str(), uuid.c_str());

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("%s_shadow_xform_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowCamXformComponent.get());

		// Shadow Perspective camera component
		mShadowCamComponent = std::make_unique<PerspCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("%s_shadow_camera_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mFieldOfView = mAngle;
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

		// Shadow Frustrum component
		mShadowFrustrumComponent = std::make_unique<RenderFrustumComponent>();
		mShadowFrustrumComponent->mID = utility::stringFormat("%s_shadow_frustrum_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowFrustrumComponent->mLineWidth = resource->mLocator.mLineWidth;
		mShadowCamEntity->mComponents.emplace_back(mShadowFrustrumComponent.get());

		// Spawn it
		if (spawnShadowCamera(*mShadowCamEntity, errorState) == nullptr)
		{
			errorState.fail("Unable to spawn directional spotlight shadow entity");
			return false;
		}

		// Push angle
		setAngle(resource->mAngle);
		return true;
	}


	void SpotLightComponentInstance::setAngle(float angle)
	{
		mAngle = angle;
		mSpawnedCamera->getComponent<PerspCameraComponentInstance>().setFieldOfView(mAngle * mFOVClip);
	}


	void SpotLightComponentInstance::setFOVClip(float clip)
	{
		mFOVClip = clip;
		mSpawnedCamera->getComponent<PerspCameraComponentInstance>().setFieldOfView(mAngle * mFOVClip);
	}
}
