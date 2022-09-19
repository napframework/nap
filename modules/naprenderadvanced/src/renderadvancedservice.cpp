/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderadvancedservice.h"

// External Includes
#include <rtti/factory.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAdvancedService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS
	
namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service
	//////////////////////////////////////////////////////////////////////////

	bool RenderAdvancedService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void RenderAdvancedService::update(double deltaTime)
	{

	}


	void RenderAdvancedService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{

	}


	void RenderAdvancedService::preShutdown()
	{

	}


	void RenderAdvancedService::registerObjectCreators(rtti::Factory& factory)
	{
		//factory.addObjectCreator(std::make_unique<>(*this));
	}
}
