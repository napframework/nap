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
	RTTI_PROPERTY("Attenuation",	&nap::SpotLightComponent::mAttenuation,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Angle",			&nap::SpotLightComponent::mAngle,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("FallOff",		&nap::SpotLightComponent::mFallOff,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
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
		registerLightUniformMember<ParameterFloat, float>(uniform::light::attenuation, resource->mAttenuation->mParameter.get(), resource->mAttenuation->getValue());
		registerLightUniformMember<ParameterFloat, float>(uniform::light::angle, resource->mAngle->mParameter.get(), resource->mAngle->getValue());
        registerLightUniformMember<ParameterFloat, float>(uniform::light::falloff, resource->mFallOff->mParameter.get(), resource->mFallOff->getValue());
		mShadowMapSize = resource->mShadowMapSize;

		return true;
	}
}
