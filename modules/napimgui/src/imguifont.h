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
	NAPAPI extern const uint nunitoSansSemiBoldSize;

	/**
	 * The compressed font data used by IMGui
	 */
	NAPAPI extern const uint nunitoSansSemiBoldData[15109];

    /**
     * 4DSOUND's chosen font.
     */
    NAPAPI extern const uint suisseIntlRegularWebTrialSize;
    NAPAPI extern const uint suisseIntlRegularWebTrialData[3848];

}
