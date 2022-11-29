/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "app.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::App)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	BaseApp::BaseApp(nap::Core& core) : mCore(core)
	{

	}


	App::App(Core& core) : BaseApp(core)
	{

	}

}
