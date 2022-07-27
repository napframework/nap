/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * A parameter is a wrapper around a value that can be specified in json.  It can be used to specify 'metadata' on top of a value, such as the min/max value. 
	 * This 'metadata' can then be used to validate the values being set on it and to offer an appropriate UI
	 *
	 * The actual data is located in derived classes; Parameter itself only serves as a base class
	 */
	class NAPAPI Parameter : public Resource
	{
		RTTI_ENABLE(Resource)
	
	public:
		/** 
		 * Set the value for this parameter from another parameter. The incoming value is guaranteed to be of the same parameter type.
		 *
		 * @param value The parameter to set the new value from
		 */
		virtual void setValue(const Parameter& value) = 0;

		/**
		 * Get the display name for this parameter. If this parameter has a name set, that name is used. Otherwise, the ID is used.
		 *
		 * @return The display name to use
		 */
		const std::string getDisplayName() const { return mName.empty() ? mID : mName; }

		std::string mName;		///< Property 'Name': The name of this property. The name is separate from the ID and doesn't have to be unique.
	};
}
