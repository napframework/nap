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
	RTTI_PROPERTY("ShadowCamera",	&nap::PointLightComponent::mShadowCamera,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowMapSize",	&nap::PointLightComponent::mShadowMapSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Attenuation",	&nap::PointLightComponent::mAttenuation,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::PointLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_PROPERTY(nap::uniform::light::attenuation, &nap::PointLightComponentInstance::mAttenuation, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// PointLightComponent
	//////////////////////////////////////////////////////////////////////////

	bool PointLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		auto* resource = getComponent<PointLightComponent>();
		mAttenuation = resource->mAttenuation;
		mShadowMapSize = resource->mShadowMapSize;
		registerUniformLightProperty(uniform::light::attenuation);
		return true;
	}
}
