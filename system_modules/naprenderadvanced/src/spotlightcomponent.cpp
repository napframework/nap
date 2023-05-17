#include "spotlightcomponent.h"

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

// nap::SpotLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SpotLightComponent)
	RTTI_PROPERTY("ShadowCamera", &nap::SpotLightComponent::mShadowCamera, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize", &nap::SpotLightComponent::mShadowMapSize, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attenuation", &nap::SpotLightComponent::mAttenuation, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Angle", &nap::SpotLightComponent::mAngle, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallOff", &nap::SpotLightComponent::mFallOff, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::SpotLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SpotLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
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
		registerLightUniformMember(uniform::light::attenuation,	resource->mAttenuation.get());
		registerLightUniformMember(uniform::light::angle,		resource->mAngle.get());
		registerLightUniformMember(uniform::light::falloff,		resource->mFallOff.get());
		mShadowMapSize = resource->mShadowMapSize;

		return true;
	}
}
