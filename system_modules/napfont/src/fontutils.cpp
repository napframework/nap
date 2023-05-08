/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "fontutils.h"

// External Includes
#include <rtti/typeinfo.h>

// Orientation enum
RTTI_BEGIN_ENUM(nap::utility::ETextOrientation)
	RTTI_ENUM_VALUE(nap::utility::ETextOrientation::Left,	"Left"),
	RTTI_ENUM_VALUE(nap::utility::ETextOrientation::Center, "Center"),
	RTTI_ENUM_VALUE(nap::utility::ETextOrientation::Right,	"Right")
RTTI_END_ENUM

namespace nap
{
	namespace utility
	{

	}
}