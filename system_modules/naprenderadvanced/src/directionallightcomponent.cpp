#include "directionallightcomponent.h"

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

// nap::DirectionalLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::DirectionalLightComponent)
	RTTI_PROPERTY("ShadowMapSize",		&nap::DirectionalLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ProjectionRect",		&nap::DirectionalLightComponent::mProjectionRect,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClippingPlanes",		&nap::DirectionalLightComponent::mClippingPlanes,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::DirectionalLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DirectionalLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool DirectionalLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		// Copy resources
		auto* resource = getComponent<DirectionalLightComponent>();
		mShadowMapSize = resource->mShadowMapSize;

		// Create shadow camera resource
		std::string uuid = math::generateUUID();
		mShadowCamEntity = std::make_unique<Entity>();
		mShadowCamEntity->mID = utility::stringFormat("%s_shadow_%s", getEntityInstance()->mID.c_str(), uuid.c_str());

		// Perspective camera component
		mShadowCamComponent = std::make_unique<OrthoCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("%s_shadow_camera_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mMode = EOrthoCameraMode::Custom;
		mShadowCamComponent->mProperties.mLeftPlane = resource->mProjectionRect.mMinPosition.x;
		mShadowCamComponent->mProperties.mRightPlane = resource->mProjectionRect.mMaxPosition.x;
		mShadowCamComponent->mProperties.mBottomPlane = resource->mProjectionRect.mMinPosition.y;
		mShadowCamComponent->mProperties.mTopPlane = resource->mProjectionRect.mMaxPosition.y;
		mShadowCamEntity->mComponents.emplace_back(mShadowCamComponent.get());

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("%s_shadow_xform_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowCamXformComponent.get());

		// Spawn shadow camera
		if (spawnShadowCamera(*mShadowCamEntity, errorState) == nullptr)
		{
			errorState.fail("Unable to spawn directional shadow camera entity");
			return false;
		}
		return true;
	}
}
