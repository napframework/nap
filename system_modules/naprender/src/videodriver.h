/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/numeric.h>
#include <string>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Video back-end options.
	 * By default, the system will try all available video back-ends in a reasonable order until it finds one that can work,
	 * but this hint allows the app or user to force a specific target, such as "x11" if, say,
	 * you are on Wayland but want to try talking to the X server instead.
	 */
	enum class EVideoDriver : int8
	{
		Default		= 0,	//< Most reasonable, first available video back-end.
		Windows		= 1,	//< Windows windowing system
		X11			= 2,	//< Linux X11 windowing system
		Wayland		= 3,	//< Linux Wayland windowing system
	};

	/**
	 * Returns video driver name
	 * @return video driver name
	 */
	NAPAPI std::string toString(EVideoDriver driver);
}
