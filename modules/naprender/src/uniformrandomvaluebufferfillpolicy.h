/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>

// Local Includes
#include "valuebufferfillpolicy.h"

namespace nap
{
	/**
	 * Typed fill policy for uniform random values. Fills buffer with random values between the specified lower bound
	 * and upper bound.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 *
	 * Fill policies can be bound to another resource's property (i.e. ValueGPUBuffer) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free to implement the way fill() is used in their own way however.
	 */
	template<typename T>
	class RandomFillPolicy : public FillPolicy<T>
	{
		RTTI_ENABLE(FillPolicy<T>)
	public:
		/**
		 * Fills the preallocated data
		 * @param numElements the number of elements to fill
		 * @param data pointer to the preallocated data
		 */
		virtual void fill(uint numElements, T* data) const override;

		T mLowerBound = T();						///< Property 'LowerBound' The lower bound of the uniform random sample
		T mUpperBound = T();						///< Property 'UpperBound' The upper bound of the uniform random sample
	};


	//////////////////////////////////////////////////////////////////////////
	// TypedRandomFillPolicyNumeric type definitions
	//////////////////////////////////////////////////////////////////////////

	using RandomFillPolicyUInt		= RandomFillPolicy<uint>;
	using RandomFillPolicyInt		= RandomFillPolicy<int>;
	using RandomFillPolicyFloat		= RandomFillPolicy<float>;
	using RandomFillPolicyVec2		= RandomFillPolicy<glm::vec2>;
	using RandomFillPolicyVec3		= RandomFillPolicy<glm::vec3>;
	using RandomFillPolicyVec4		= RandomFillPolicy<glm::vec4>;
	using RandomFillPolicyMat4		= RandomFillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void RandomFillPolicy<T>::fill(uint numElements, T* data) const
	{
		for (uint i = 0; i < numElements; i++)
		{
			*data = math::random(mLowerBound, mUpperBound);
			++data;
		}
	}
}
