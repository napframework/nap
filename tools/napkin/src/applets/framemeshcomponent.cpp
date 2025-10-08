/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "framemeshcomponent.h"

// External Includes
#include <entity.h>

// nap::framemeshcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::FrameMeshComponent)
	// Put additional properties here
RTTI_END_CLASS

// nap::framemeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::FrameMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	bool FrameMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void FrameMeshComponentInstance::update(double deltaTime)
	{

	}
}
