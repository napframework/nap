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
	RTTI_PROPERTY("ShadowCamera", &nap::DirectionalLightComponent::mShadowCamera, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize", &nap::DirectionalLightComponent::mShadowMapSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attenuation", &nap::DirectionalLightComponent::mAttenuation, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::DirectionalLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DirectionalLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// DirectionalLightComponent
	//////////////////////////////////////////////////////////////////////////

	bool DirectionalLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		auto* resource = getComponent<DirectionalLightComponent>();
		registerLightUniformMember(uniform::light::attenuation, resource->mAttenuation.get());
		mShadowMapSize = resource->mShadowMapSize;

		return true;
	}
}
