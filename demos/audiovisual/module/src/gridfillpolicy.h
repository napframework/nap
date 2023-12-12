/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>

// Local Includes
#include "fillpolicy.h"

namespace nap
{
	/**
	 * Typed fill policy for uniform random values. Fills buffer with random values between the specified lower bound
	 * and upper bound.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 *
	 * Fill policies can be bound to another resource's property (i.e. GPUBufferNumeric) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free to implement the way fill() is used in their own way however.
	 */
	class GridFillPolicy : public FillPolicyVec4
	{
		RTTI_ENABLE(FillPolicyVec4)
	public:
		/**
		 * 
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Fills the preallocated data
		 * @param numElements the number of elements to fill
		 * @param data pointer to the preallocated data
		 */
		virtual void fill(uint numElements, glm::vec4* data) const override;

		glm::vec2 mSize = { 1.0, 1.0 };
		uint mRows = 1;
		uint mColumns = 1;
	};
}
