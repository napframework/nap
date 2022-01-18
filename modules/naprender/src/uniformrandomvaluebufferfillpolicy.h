/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <nap/numeric.h>

// Local Includes
#include "structbufferfillpolicy.h"
#include "valuebufferfillpolicy.h"

namespace nap
{
	/**
	 * 
	 */
	template<typename T>
	class UniformRandomValueBufferFillPolicy : public TypedValueBufferFillPolicy<T>
	{
		RTTI_ENABLE(TypedValueBufferFillPolicy<T>)
	public:
		/**
		 *
		 */
		virtual void fill(uint numElements, T* data) const override;

		T mLowerBound = T();						///< Property 'LowerBound'
		T mUpperBound = T();						///< Property 'UpperBound'
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformRandomBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

	using UniformRandomUIntBufferFillPolicy = UniformRandomValueBufferFillPolicy<uint>;
	using UniformRandomIntBufferFillPolicy = UniformRandomValueBufferFillPolicy<int>;
	using UniformRandomFloatBufferFillPolicy = UniformRandomValueBufferFillPolicy<float>;
	using UniformRandomVec2BufferFillPolicy = UniformRandomValueBufferFillPolicy<glm::vec2>;
	using UniformRandomVec3BufferFillPolicy = UniformRandomValueBufferFillPolicy<glm::vec3>;
	using UniformRandomVec4BufferFillPolicy = UniformRandomValueBufferFillPolicy<glm::vec4>;
	using UniformRandomMat4BufferFillPolicy = UniformRandomValueBufferFillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void UniformRandomValueBufferFillPolicy<T>::fill(uint numElements, T* data) const
	{
		for (uint i = 0; i < numElements; i++)
		{
			*data = math::random(mLowerBound, mUpperBound);
			++data;
		}
	}
}
