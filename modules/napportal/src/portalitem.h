/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents a single item (e.g. parameter, button, stream) in a NAP portal.
	 */
	class NAPAPI PortalItem : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		// Default destructor
		virtual ~PortalItem();
	};
}
