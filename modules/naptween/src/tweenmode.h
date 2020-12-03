/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	/**
	 * All tween mode available
	 */
	enum ETweenMode : int
	{
		NORMAL 		= 0,
		LOOP		= 1,
		PING_PONG	= 2,
		REVERSE		= 3
	};
}