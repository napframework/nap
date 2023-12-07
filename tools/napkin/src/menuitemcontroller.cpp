/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "menuitemcontroller.h"

namespace napkin
{
	void MenuItemController::populate(RTTIItem& item, QMenu& menu)
	{
		for (const auto& action : mOptions)
		{
			action.mCallback(item, menu);
		}
	}
}
