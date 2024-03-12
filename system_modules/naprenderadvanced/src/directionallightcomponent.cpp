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
	RTTI_PROPERTY("ProjectionSize",		&nap::DirectionalLightComponent::mProjectionSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize",		&nap::DirectionalLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
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

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("%s_shadow_xform_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowCamXformComponent.get());

		// Perspective camera component
		float projection = resource->mProjectionSize / 2.0f;
		mShadowCamComponent = std::make_unique<OrthoCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("%s_shadow_camera_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mMode = EOrthoCameraMode::Custom;
		mShadowCamComponent->mProperties.mLeftPlane = -projection;
		mShadowCamComponent->mProperties.mRightPlane = projection;
		mShadowCamComponent->mProperties.mBottomPlane = -projection;
		mShadowCamComponent->mProperties.mTopPlane = projection;
		mShadowCamEntity->mComponents.emplace_back(mShadowCamComponent.get());

		// Shadow Origin component
		mGnomonMesh = std::make_unique<GnomonMesh>(*getEntityInstance()->getCore());
		mGnomonMesh->mID = utility::stringFormat("%s_shadow_gnomon_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mGnomonMesh->mSize = resource->mLocator.mGnomonSize;
		if (!mGnomonMesh->init(errorState))
			return false;

		mShadowOriginComponent = std::make_unique<RenderGnomonComponent>();
		mShadowOriginComponent->mID = utility::stringFormat("%s_shadow_origin_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowOriginComponent->mDepthMode = EDepthMode::ReadOnly;
		mShadowOriginComponent->mMesh = mGnomonMesh.get();
		mShadowOriginComponent->mLineWidth = resource->mLocator.mLineWidth;
		mShadowCamEntity->mComponents.emplace_back(mShadowOriginComponent.get());

		// Shadow Frustrum component
		mShadowFrustrumComponent = std::make_unique<RenderFrustumComponent>();
		mShadowFrustrumComponent->mID = utility::stringFormat("%s_shadow_frustrum_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowFrustrumComponent.get());

		// Spawn shadow camera
		if (spawnShadowCamera(*mShadowCamEntity, errorState) == nullptr)
		{
			errorState.fail("Unable to spawn directional shadow camera entity");
			return false;
		}
		return true;
	}
}
