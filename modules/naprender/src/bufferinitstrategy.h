/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <nap/numeric.h>

// Local Includes
#include "mathutils.h"

namespace nap
{
	/**
	 *
	 */
	class NAPAPI BaseValueBufferInitStrategy : public Resource
	{
		RTTI_ENABLE(Resource)
	};


	/**
	 *
	 */
	template<typename T>
	class NAPAPI TypedValueBufferInitStrategy : public BaseValueBufferInitStrategy
	{
		RTTI_ENABLE(BaseValueBufferInitStrategy)
	public:
		virtual bool initBuffer(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) = 0;
	};


	/**
	 *
	 */
	template<typename T>
	class ConstantBufferInitStrategy : public TypedValueBufferInitStrategy<T>
	{
		RTTI_ENABLE(TypedValueBufferInitStrategy<T>)
	public:
		virtual bool initBuffer(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) override;

		T mConstant = T();			///< Property 'Constant'
	};


	/**
	 *
	 */
	template<typename T>
	class UniformRandomBufferInitStrategy : public TypedValueBufferInitStrategy<T>
	{
		RTTI_ENABLE(TypedValueBufferInitStrategy<T>)
	public:
		virtual bool initBuffer(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) override;

		T mLowerBound = T();		///< Property 'LowerBound'
		T mUpperBound = T();		///< Property 'UpperBound'
	};


	//////////////////////////////////////////////////////////////////////////
	// BaseValueBufferInitStrategy type definitions
	//////////////////////////////////////////////////////////////////////////

	using IntBufferInitStrategy = TypedValueBufferInitStrategy<int>;
	using FloatBufferInitStrategy = TypedValueBufferInitStrategy<float>;
	using Vec2BufferInitStrategy = TypedValueBufferInitStrategy<glm::vec2>;
	using Vec3BufferInitStrategy = TypedValueBufferInitStrategy<glm::vec3>;
	using Vec4BufferInitStrategy = TypedValueBufferInitStrategy<glm::vec4>;
	using Mat4BufferInitStrategy = TypedValueBufferInitStrategy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// UniformRandomBufferInitStrategy type definitions
	//////////////////////////////////////////////////////////////////////////

	using UniformRandomIntBufferInitStrategy = UniformRandomBufferInitStrategy<int>;
	using UniformRandomFloatBufferInitStrategy = UniformRandomBufferInitStrategy<float>;
	using UniformRandomVec2BufferInitStrategy = UniformRandomBufferInitStrategy<glm::vec2>;
	using UniformRandomVec3BufferInitStrategy = UniformRandomBufferInitStrategy<glm::vec3>;
	using UniformRandomVec4BufferInitStrategy = UniformRandomBufferInitStrategy<glm::vec4>;
	using UniformRandomMat4BufferInitStrategy = UniformRandomBufferInitStrategy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// ConstantBufferInitStrategy type definitions
	//////////////////////////////////////////////////////////////////////////

	using ConstantIntBufferInitStrategy = ConstantBufferInitStrategy<int>;
	using ConstantFloatBufferInitStrategy = ConstantBufferInitStrategy<float>;
	using ConstantVec2BufferInitStrategy = ConstantBufferInitStrategy<glm::vec2>;
	using ConstantVec3BufferInitStrategy = ConstantBufferInitStrategy<glm::vec3>;
	using ConstantVec4BufferInitStrategy = ConstantBufferInitStrategy<glm::vec4>;
	using ConstantMat4BufferInitStrategy = ConstantBufferInitStrategy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool ConstantBufferInitStrategy<T>::initBuffer(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState)
	{
		stagingBuffer.resize(numElements, mConstant);
		return true;
	}


	template<typename T>
	bool UniformRandomBufferInitStrategy<T>::initBuffer(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState)
	{
		stagingBuffer.reserve(numElements);

		for (size_t i = 0; i < numElements; i++)
			stagingBuffer.emplace_back(math::random<T>(mLowerBound, mUpperBound));

		return true;
	}
}
