/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "parametersimple.h"


namespace nap
{
	class ParameterString : public ParameterSimple<std::string>
	{
		RTTI_ENABLE(ParameterSimple<std::string>)

	public:

		/**
		 * Allocated the requested size for the string
		 * @param error contains the error message when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;

		size_t mSize = 255;		///< Property: 'Size' the allocated size for the string
	};
}
