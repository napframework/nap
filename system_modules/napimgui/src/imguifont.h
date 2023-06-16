/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Size of the font data buffer
	 */
	inline constexpr uint manropeMediumSize = 80368;

	/**
	 * The compressed font data used by IMGui
	 */
	extern const uint manropeMediumData[manropeMediumSize / 4];
}
