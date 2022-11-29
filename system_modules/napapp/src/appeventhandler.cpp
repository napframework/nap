/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "appeventhandler.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AppEventHandler)
	RTTI_CONSTRUCTOR(nap::BaseApp&)
RTTI_END_CLASS	

namespace nap
{
	AppEventHandler::AppEventHandler(BaseApp& app) : mApp(app)
	{ }
}