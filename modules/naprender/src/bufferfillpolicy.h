/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <nap/numeric.h>

// Local Includes
#include "mathutils.h"
#include "structbufferdescriptor.h"

namespace nap
{
	class Uniform;

	//////////////////////////////////////////////////////////////////////////
	// StructBufferFillPolicies
	//////////////////////////////////////////////////////////////////////////

	using FillPolicyFunction = std::function<void(const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)>;

	class NAPAPI BaseFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseFillPolicy() = default;

		virtual ~BaseFillPolicy() = default;

		virtual bool init(utility::ErrorState& errorState) override;

		bool fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState);

	protected:
		virtual bool registerFillPolicyFunction(rtti::TypeInfo type, FillPolicyFunction fillFunction);
		std::unordered_map<rtti::TypeInfo, FillPolicyFunction> mFillMap;

	private:
		int fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data);

		template<typename UNIFORMTYPE, typename VALUETYPE>
		void setValues(UNIFORMTYPE* lowerBoundUniform, UNIFORMTYPE* upperBoundUniform, int count, uint8* data)
		{
			auto it = mFillMap.find(RTTI_OF(VALUETYPE));
			assert(it != mFillMap.end());

			for (int idx = 0; idx < count; idx++)
				it->second(lowerBoundUniform, upperBoundUniform, (uint8*)(data + sizeof(VALUETYPE) * idx));
		};
	};


	/**
	 * 
	 */
	class NAPAPI UniformRandomFillPolicy : public BaseFillPolicy
	{
		RTTI_ENABLE(BaseFillPolicy)
	public:
		UniformRandomFillPolicy() = default;

		virtual bool init(utility::ErrorState& errorState) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// ValueBufferFillPolicies
	//////////////////////////////////////////////////////////////////////////

	/**
	 *
	 */
	class NAPAPI BaseValueBufferFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	};


	/**
	 *
	 */
	template<typename T>
	class NAPAPI TypedValueBufferFillPolicy : public BaseValueBufferFillPolicy
	{
		RTTI_ENABLE(BaseValueBufferFillPolicy)
	public:
		virtual bool fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) = 0;
	};


	/**
	 *
	 */
	template<typename T>
	class ConstantValueBufferFillPolicy : public TypedValueBufferFillPolicy<T>
	{
		RTTI_ENABLE(TypedValueBufferFillPolicy<T>)
	public:
		virtual bool fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState) override;

		T mConstant = T();			///< Property 'Constant'
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

		T mLowerBound = T();		///< Property 'LowerBound'
		T mUpperBound = T();		///< Property 'UpperBound'
	};


	//////////////////////////////////////////////////////////////////////////
	// BaseValueBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

	using IntBufferFillPolicy = TypedValueBufferFillPolicy<int>;
	using FloatBufferFillPolicy = TypedValueBufferFillPolicy<float>;
	using Vec2BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec2>;
	using Vec3BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec3>;
	using Vec4BufferFillPolicy = TypedValueBufferFillPolicy<glm::vec4>;
	using Mat4BufferFillPolicy = TypedValueBufferFillPolicy<glm::mat4>;


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
	// ConstantBufferFillPolicy type definitions
	//////////////////////////////////////////////////////////////////////////

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
	bool ConstantValueBufferFillPolicy<T>::fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState)
	{
		stagingBuffer.resize(numElements, mConstant);
		return true;
	}


	template<typename T>
	bool UniformRandomValueBufferFillPolicy<T>::fill(uint numElements, std::vector<T>& stagingBuffer, utility::ErrorState& errorState)
	{
		stagingBuffer.reserve(numElements);

		for (size_t i = 0; i < numElements; i++)
			stagingBuffer.emplace_back(math::random<T>(mLowerBound, mUpperBound));

		return true;
	}
}
