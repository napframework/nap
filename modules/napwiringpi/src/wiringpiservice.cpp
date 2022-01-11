/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "wiringpiservice.h"

// Third party includes
#include <wiringPi.h>

#include <nap/logger.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WiringPiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    WiringPiService::WiringPiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool WiringPiService::init(nap::utility::ErrorState& errorState)
    {
        wiringPiSetup () ;
        pinMode (0, OUTPUT) ;

        digitalWrite (0, HIGH) ; delay (500) ;
        digitalWrite (0,  LOW) ; delay (500) ;

        return true;
    }
    
    
    void WiringPiService::registerObjectCreators(rtti::Factory& factory)
    {

    }


    void WiringPiService::update(double deltaTime)
    {
    }

}
