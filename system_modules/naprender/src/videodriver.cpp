/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "videodriver.h"

// External includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::EVideoDriver)
	RTTI_ENUM_VALUE(nap::EVideoDriver::Default,		"System Default"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::Windows,		"Windows"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::X11,			"X11"),
	RTTI_ENUM_VALUE(nap::EVideoDriver::Wayland,		"Wayland")
RTTI_END_ENUM

namespace  nap
{
	std::string toString(EVideoDriver driver)
	{
		return RTTI_OF(EVideoDriver).get_enumeration().value_to_name(driver).to_string();
	}
}