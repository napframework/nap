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

		// Copy resource properties
		auto* resource = getComponent<SpotLightComponent>();
		mAttenuation = resource->mAttenuation;
		mAngle = resource->mAngle;
		mFalloff = resource->mFalloff;
		mShadowMapSize = resource->mShadowMapSize;

		// Register uniforms to auto push
		registerUniformLightProperty(uniform::light::attenuation);
		registerUniformLightProperty(uniform::light::angle);
		registerUniformLightProperty(uniform::light::falloff);

		// Bail if we're not using shadows
		if (!mResource->mCastShadows)
			return true;

		// Create and add spotlight shadow map entity
		auto* scene_service = getEntityInstance()->getCore()->getService<SceneService>(); 
		assert(scene_service != nullptr);
		if (!errorState.check(!scene_service->getScenes().empty(),
			"Unable to create spotlight shadow entity, no scene available"))
			return false;

		// Create shadow camera resource
		nap::Entity shadow_camera_entity;
		shadow_camera_entity.mID = utility::stringFormat("SpotShadowEntity_%s", math::generateUUID().c_str());

		// Perspective camera component
		mShadowCamComponent = std::make_unique<PerspCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("SpotShadowCamera_%s", math::generateUUID().c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mFieldOfView = mAngle;
		shadow_camera_entity.mComponents.emplace_back(mShadowCamComponent.get());

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("SpotShadowTransform_%s", math::generateUUID().c_str());
		shadow_camera_entity.mComponents.emplace_back(mShadowCamXformComponent.get());

		// Spawn it
		mScene = std::make_unique<nap::Scene>(*getEntityInstance()->getCore());
		mSpawnedCameraEntity = mScene->spawn(shadow_camera_entity, errorState);

		// Check if it was created
		if (!errorState.check(mSpawnedCameraEntity != nullptr, "Unable to spawn spotlight shadow entity"))
			return false;

		return true;
	}


	void SpotLightComponentInstance::onDestroy()
	{
		if (mSpawnedCameraEntity != nullptr)
		{
			assert(mScene != nullptr);
			mScene->destroy(mSpawnedCameraEntity);
		}
		LightComponentInstance::onDestroy();
	}


	void SpotLightComponentInstance::update(double deltaTime)
	{
		LightComponentInstance::update(deltaTime);
		if (mSpawnedCameraEntity != nullptr)
		{
			mSpawnedCameraEntity->getComponent<TransformComponentInstance>().setLocalTransform(getTransform().getGlobalTransform());
		}
	}


	nap::CameraComponentInstance* SpotLightComponentInstance::getShadowCamera() const
	{
		if (mSpawnedCameraEntity != nullptr)
		{
			return &mSpawnedCameraEntity->getComponent<CameraComponentInstance>();
		}
		return nullptr;
	}


	nap::CameraComponentInstance* SpotLightComponentInstance::getShadowCamera()
	{
		if (mSpawnedCameraEntity != nullptr)
		{
			return &mSpawnedCameraEntity->getComponent<CameraComponentInstance>();
		}
		return nullptr;
	}

}
