#include "pointlightcomponent.h"

// External Includes
#include "entity.h"

RTTI_BEGIN_CLASS(nap::PointlightComponent)
	RTTI_PROPERTY("Intensity", &nap::PointlightComponent::mIntensity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Attenuation", &nap::PointlightComponent::mAttenuation, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointlightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	bool PointlightComponentInstance::init(utility::ErrorState& errorState)
	{
		mIntensity = getComponent<PointlightComponent>()->mIntensity;
		mAttenuation = getComponent<PointlightComponent>()->mAttenuation;
		return true;
	}
}