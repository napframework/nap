/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include<rtti/objectptr.h>

namespace nap
{
	/**
	 * Can be used to point to other resources in a json file
	 * This is just a regular object ptr but allows for a more explicit definition of a link
	 */
	template<typename T>
	using ResourcePtr = rtti::ObjectPtr<T>;
}