/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	namespace utility 
	{
		/**
		 * Controls the horizontal text draw orientation, vertical alignment is based on the character origin line
		 * This enum is serializable and can be used as a property
		 */
		enum class ETextOrientation : int
		{
			Left	= 0,	///< Draws text to the right of the horizontal coordinate
			Center	= 1,	///< Centers the text around the horizontal coordinate
			Right	= 2		///< Draws the text to the left of the horizontal coordinate
		};
	}
}
