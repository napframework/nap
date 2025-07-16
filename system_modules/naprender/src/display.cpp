/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "display.h"
#include "sdlhelpers.h"

namespace nap
{
	Display::Display(int index) : mIndex(index)
	{
		if (!SDL::getDisplayName(index, mName))
			return;

		if (!SDL::getDisplayBounds(index, mMin, mMax))
			return;

		if (!SDL::getDisplayContentScale(index, &mScale))
			return;

		mDPI = mScale * 96.0f;
		mValid = true;
	}


	Display::Display(SDL_Window* window) : Display(SDL::getDisplayIndex(window))
	{ }


	std::string nap::Display::toString() const
	{
		return utility::stringFormat
		(
			"Display: %d, %s, scale: %.1f, min: %d-%d, max: %d-%d",
			mIndex,
			mName.c_str(),
			mScale,
			mMin.x, mMin.y,
			mMax.x, mMax.y
		);
	}
}
