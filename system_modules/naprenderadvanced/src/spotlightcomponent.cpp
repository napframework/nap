#include "spotlightcomponent.h"

// Local includes
#include "renderadvancedservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <transformcomponent.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>

// nap::SpotLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SpotLightComponent)
	RTTI_PROPERTY("ShadowCamera",	&nap::SpotLightComponent::mShadowCamera,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize",	&nap::SpotLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attenuation",	&nap::SpotLightComponent::mAttenuation,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Angle",			&nap::SpotLightComponent::mAngle,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FallOff",		&nap::SpotLightComponent::mFallOff,			nap::rtti::EPropertyMetaData::Default)
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
		registerLightUniformMember<ParameterFloat, float>(uniform::light::attenuation, nullptr, resource->mAttenuation);
		registerLightUniformMember<ParameterFloat, float>(uniform::light::angle, nullptr, resource->mAngle);
        registerLightUniformMember<ParameterFloat, float>(uniform::light::falloff, nullptr, resource->mFallOff);
		mShadowMapSize = resource->mShadowMapSize;

		return true;
	}
}
