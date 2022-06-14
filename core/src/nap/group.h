/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "resource.h"
#include "resourceptr.h"
#include "rtti/objectptr.h"
#include "resource.h"

namespace nap
{
	/**
	 * Groups together a set of resources.
	 */
	class NAPAPI Group : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::vector<rtti::ObjectPtr<Resource>> mResources;		///< Property: 'Resources' The resources that belong to this group
	};
}
