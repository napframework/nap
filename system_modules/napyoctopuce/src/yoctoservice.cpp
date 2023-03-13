/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "yoctoservice.h"

// External includes
#include <yocto_api.h>
#include <memory>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::YoctoService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	YoctoService::YoctoService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	void YoctoService::shutdown()
	{
		yFreeAPI();
	}
}