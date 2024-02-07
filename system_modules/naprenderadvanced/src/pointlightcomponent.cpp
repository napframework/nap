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
RTTI_BEGIN_CLASS(nap::PointLightComponent)
	RTTI_PROPERTY("Attenuation",	&nap::PointLightComponent::mAttenuation,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FieldOfView",	&nap::PointLightComponent::mFieldOfView,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClippingPlanes",	&nap::PointLightComponent::mClippingPlanes,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize",	&nap::PointLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
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

		// Perspective camera component
		mShadowCamComponent = std::make_unique<PerspCameraComponent>();
		mShadowCamComponent->mID = utility::stringFormat("%s_shadow_camera_%s",getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamComponent->mProperties.mNearClippingPlane = resource->mClippingPlanes[0];
		mShadowCamComponent->mProperties.mFarClippingPlane = resource->mClippingPlanes[1];
		mShadowCamComponent->mProperties.mFieldOfView = resource->mFieldOfView;
		mShadowCamEntity->mComponents.emplace_back(mShadowCamComponent.get());

		// Transform component
		mShadowCamXformComponent = std::make_unique<TransformComponent>();
		mShadowCamXformComponent->mID = utility::stringFormat("%s_shadow_xform_%s", getEntityInstance()->mID.c_str(), uuid.c_str());
		mShadowCamEntity->mComponents.emplace_back(mShadowCamXformComponent.get());

		// Spawn it
		if (spawnShadowCamera(*mShadowCamEntity, errorState) == nullptr)
		{
			errorState.fail("Unable to spawn directional spotlight shadow entity");
			return false;
		}
		return true;
	}
}
