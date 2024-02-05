#include "spotlightcomponent.h"

// Local includes
#include "renderadvancedservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <transformcomponent.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>
#include <sceneservice.h>
#include <scene.h>

// nap::SpotLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SpotLightComponent)
	RTTI_PROPERTY("Attenuation",			&nap::SpotLightComponent::mAttenuation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Angle",					&nap::SpotLightComponent::mAngle,			nap::rtti::EPropertyMetaData::Default)
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
	//////////////////////////////////////////////////////////////////////////
	// SpotLightComponent
	//////////////////////////////////////////////////////////////////////////

	bool SpotLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		auto* resource = getComponent<SpotLightComponent>();
		mAttenuation = resource->mAttenuation;
		mAngle = resource->mAngle;
		mFalloff = resource->mFalloff;
		mShadowMapSize = resource->mShadowMapSize;

		// Create and add shadow entity
		mSceneService = getEntityInstance()->getCore()->getService<SceneService>();
		assert(mSceneService != nullptr);
		const auto& scenes = mSceneService->getScenes();
		if (!errorState.check(!scenes.empty(), "Unable to create shadow camera entity, no scene available"))
			return false;

		// Create shadow camera resource
		auto* scene_resource = *scenes.begin();
		nap::Entity shadow_camera_entity;
		shadow_camera_entity.mID = utility::stringFormat("SpotShadowEntity_%s", math::generateUUID().c_str());

		mShadowCamComponent = std::make_unique<PerspCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("SpotShadowCamera_%s", math::generateUUID().c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mFieldOfView = mAngle;
		shadow_camera_entity.mComponents.emplace_back(mShadowCamComponent.get());

		// Create transform
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("SpotShadowTransform_%s", math::generateUUID().c_str());
		shadow_camera_entity.mComponents.emplace_back(mShadowCamXformComponent.get());

		// Spawn it
		mSpawnedCameraEntity = scene_resource->spawn(shadow_camera_entity, errorState);
		if (!errorState.check(mSpawnedCameraEntity != nullptr, "Unable to create shadow camera entity, spawning process failed"))
			return false;

		registerUniformLightProperty(uniform::light::attenuation);
		registerUniformLightProperty(uniform::light::angle);
		registerUniformLightProperty(uniform::light::falloff);
		return true;
	}

	
	void SpotLightComponentInstance::update(double deltaTime)
	{
		// Set light position
		LightComponentInstance::update(deltaTime);
		auto& lamp_xform = mSpawnedCameraEntity->getComponent<nap::TransformComponentInstance>();
		lamp_xform.setLocalTransform(getTransform().getGlobalTransform());
	}


	nap::CameraComponentInstance* SpotLightComponentInstance::getShadowCamera()
	{
		auto& comp = mSpawnedCameraEntity->getComponent<PerspCameraComponentInstance>();
		return &comp;
		//return (mShadowCamera != nullptr) ? &(*mShadowCamera) : nullptr;
	}
}
