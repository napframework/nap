/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>

namespace nap
{
	/**
	 * All possible mouse buttons
	 */
	enum class EMouseButton : int
	{
		UNKNOWN		= -1,
		LEFT		= 0,
		MIDDLE		= 1,
		RIGHT		= 2
	};
}
