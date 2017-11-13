// Local includes
#include "laseroscinputcomponent.h"
#include "oscservice.h"

// External includes
#include <nap/entity.h>
#include <nap/core.h>
#include <iostream>

RTTI_BEGIN_CLASS(nap::LaserOSCInputComponent)
	RTTI_PROPERTY("LaserID",	&nap::LaserOSCInputComponent::mLaserID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserOSCInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LaserOSCInputComponentInstance::init(utility::ErrorState& errorState)
	{
		// Init base first (otherwise address filter is overwritten)
		if (!OSCInputComponentInstance::init(errorState))
			return false;

		// Build custom address filter based on laser id
		for (const std::string& address : getComponent<LaserOSCInputComponent>()->mAddressFilter)
		{
			std::string address_filter = utility::stringFormat("/%d/%s", getComponent<LaserOSCInputComponent>()->mLaserID, address.c_str());
			mAddressFilter.emplace_back(address_filter);
		}
		
		return true;
	}
}