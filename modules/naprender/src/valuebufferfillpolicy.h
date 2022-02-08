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
	 * Base class of fill policies for value buffers.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 * 
	 * Fill policies can be bound to another resource's property (i.e. ValueGPUBuffer) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free implement the way fill() is used in their own way however.
	 */
	class NAPAPI BaseValueBufferFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseValueBufferFillPolicy() = default;
		virtual ~BaseValueBufferFillPolicy() = default;
	};


	/**
	 * Base class of a fill policy implementation (e.g. ConstantValueBufferFillPolicy<T>) of a specific value type T.
	 * Inherit from this class in you intend to implement your own value fill policy.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 *
	 * Fill policies can be bound to another resource's property (i.e. ValueGPUBuffer) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free to implement the way fill() is used in their own way however.
	 */
	template<typename T>
	class NAPAPI TypedValueBufferFillPolicy : public BaseValueBufferFillPolicy
	{
		RTTI_ENABLE(BaseValueBufferFillPolicy)
	public:
		/**
		 * Fills the preallocated data
		 * @param numElements the number of elements to fill
		 * @param data pointer to the preallocated data
		 */
		virtual void fill(uint numElements, T* data) const = 0;
	};


	/**
	 * Typed fill policy for constant values. Fills buffer with a specified constant.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 *
	 * Fill policies can be bound to another resource's property (i.e. ValueGPUBuffer) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free to implement the way fill() is used in their own way however.
	 */
	template<typename T>
	class ConstantValueBufferFillPolicy : public TypedValueBufferFillPolicy<T>
	{
		RTTI_ENABLE(TypedValueBufferFillPolicy<T>)
	public:
		/**
		 * Fills the preallocated data
		 * @param numElements the number of elements to fill
		 * @param data pointer to the preallocated data
		 */
		virtual void fill(uint numElements, T* data) const override;

		T mConstant = T();			///< Property 'Constant'
	};


	//////////////////////////////////////////////////////////////////////////
	// BaseValueBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

	using UIntBufferFillPolicy = TypedValueBufferFillPolicy<uint>;
	using IntBufferFillPolicy = TypedValueBufferFillPolicy<int>;
	using FloatBufferFillPolicy = TypedValueBufferFillPolicy<float>;
	using Vec2BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec2>;
	using Vec3BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec3>;
	using Vec4BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec4>;
	using Mat4BufferFillPolicy = TypedValueBufferFillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// ConstantBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

	using ConstantUIntBufferFillPolicy = ConstantValueBufferFillPolicy<uint>;
	using ConstantIntBufferFillPolicy = ConstantValueBufferFillPolicy<int>;
	using ConstantFloatBufferFillPolicy = ConstantValueBufferFillPolicy<float>;
	using ConstantVec2BufferFillPolicy = ConstantValueBufferFillPolicy<glm::vec2>;
	using ConstantVec3BufferFillPolicy = ConstantValueBufferFillPolicy<glm::vec3>;
	using ConstantVec4BufferFillPolicy = ConstantValueBufferFillPolicy<glm::vec4>;
	using ConstantMat4BufferFillPolicy = ConstantValueBufferFillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ConstantValueBufferFillPolicy<T>::fill(uint numElements, T* data) const 
	{
		for (uint i = 0; i < numElements; i++)
		{
			*data = mConstant;
			++data;
		}
	}
}
