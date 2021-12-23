/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <nap/numeric.h>

// Local Includes
#include "structbufferfillpolicy.h"

namespace nap
{
	// Forward declares
	class Uniform;

	/**
	 * 
	 */
	class NAPAPI UniformRandomStructBufferFillPolicy : public BaseStructBufferFillPolicy
	{
		RTTI_ENABLE(BaseStructBufferFillPolicy)
	public:
		UniformRandomStructBufferFillPolicy() = default;

		virtual bool init(utility::ErrorState& errorState) override;

		virtual bool fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState) override;

		ResourcePtr<UniformStruct> mLowerBound;		///< Property 'LowerBound'
		ResourcePtr<UniformStruct> mUpperBound;		///< Property 'UpperBound'
	};
}
