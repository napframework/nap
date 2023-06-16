/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "apivalue.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents the signature of a method.
	 * This object is a resource that can be serialized from and to an external environment.
	 */
	class NAPAPI APISignature : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default destructor
		virtual ~APISignature();

		/**
		 * @return number of arguments associated with this api signature	
		 */
		int getCount() const								{ return static_cast<int>(mArguments.size()); }

		/**
		 * @param index the index to get the api value for
		 * @return the value at the given index
		 */
		const APIBaseValue& getValue(int index)	const		{ return *(mArguments[index]); }

		std::vector<ResourcePtr<APIBaseValue>> mArguments;	///< Property: 'Arguments': All input arguments associated with this signature
	};
}
