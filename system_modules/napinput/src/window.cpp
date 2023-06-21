/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "window.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Window)
RTTI_END_CLASS

namespace nap
{
	void Window::addEvent(WindowEventPtr inEvent)
	{
		mWindowEvents.emplace_back(std::move(inEvent));
	}

	void Window::processEvents()
	{
		for (auto& event : mWindowEvents)
			mWindowEvent(*event);

		mWindowEvents.clear();
	}
}