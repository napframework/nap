/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <nap/numeric.h>

// Local Includes
#include "bufferfillpolicy.h"

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


	/**
	 *
	 */
	template<typename T>
	class UniformRandomValueBufferFillPolicy : public TypedValueBufferFillPolicy<T>
	{
		RTTI_ENABLE(TypedValueBufferFillPolicy<T>)
	public:
		virtual bool fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) override;

		T mLowerBound = T();						///< Property 'LowerBound'
		T mUpperBound = T();						///< Property 'UpperBound'
	};


	//////////////////////////////////////////////////////////////////////////
	// UniformRandomBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

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
	bool UniformRandomValueBufferFillPolicy<T>::fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState)
	{
		stagingBuffer.reserve(numElements);
		for (uint idx = 0; idx < numElements; idx++)
			stagingBuffer.emplace_back(math::random(mLowerBound, mUpperBound));
		
		return true;
	}
}
