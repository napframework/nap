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
	 * Base class of fill policies for numeric buffers.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 * 
	 * Fill policies can be bound to another resource's property (i.e. GPUBufferNumeric) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free implement the way fill() is used in their own way however.
	 */
	class NAPAPI BaseFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseFillPolicy() = default;
		virtual ~BaseFillPolicy() = default;
	};


	/**
	 * Base class of a fill policy implementation (e.g. ConstantFillPolicy<T>) of a specific value type T.
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
	class NAPAPI FillPolicy : public BaseFillPolicy
	{
		RTTI_ENABLE(BaseFillPolicy)
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
	class ConstantFillPolicy : public FillPolicy<T>
	{
		RTTI_ENABLE(FillPolicy<T>)
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
	// BaseFillPolicyNumeric type definitions
	//////////////////////////////////////////////////////////////////////////

	using FillPolicyUInt				= FillPolicy<uint>;
	using FillPolicyInt					= FillPolicy<int>;
	using FillPolicyFloat				= FillPolicy<float>;
	using FillPolicyVec2				= FillPolicy<glm::vec2>;
	using FillPolicyVec3				= FillPolicy<glm::vec3>;
	using FillPolicyVec4				= FillPolicy<glm::vec4>;
	using FillPolicyMat4				= FillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// ConstantBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

	using ConstantFillPolicyUInt		= ConstantFillPolicy<uint>;
	using ConstantFillPolicyInt			= ConstantFillPolicy<int>;
	using ConstantFillPolicyFloat		= ConstantFillPolicy<float>;
	using ConstantFillPolicyVec2		= ConstantFillPolicy<glm::vec2>;
	using ConstantFillPolicyVec3		= ConstantFillPolicy<glm::vec3>;
	using ConstantFillPolicyVec4		= ConstantFillPolicy<glm::vec4>;
	using ConstantFillPolicyMat4		= ConstantFillPolicy<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ConstantFillPolicy<T>::fill(uint numElements, T* data) const 
	{
		for (uint i = 0; i < numElements; i++)
		{
			*data = mConstant;
			++data;
		}
	}
}
