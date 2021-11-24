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
	// Forward declares
	class UniformValue;

	//////////////////////////////////////////////////////////////////////////
	// StructBufferFillPolicy
	//////////////////////////////////////////////////////////////////////////

	using FillValueFunction = std::function<void(const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)>;

	class NAPAPI BaseStructBufferFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseStructBufferFillPolicy() = default;
		virtual ~BaseStructBufferFillPolicy() = default;

		/**
		 * 
		 */
		virtual bool fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState);

	protected:
		/**
		 * 
		 */
		bool registerFillPolicyFunction(rtti::TypeInfo type, FillValueFunction fillValueFunction);

		/**
		 * 
		 */
		size_t fillFromUniformRecursive(const UniformStruct* uniformStruct, const UniformStruct* referenceUniformStructA, const UniformStruct* referenceUniformStructB, uint8* data);

		std::unordered_map<rtti::TypeInfo, FillValueFunction> mFillValueFunctionMap;

	private:
		template<typename T>
		void setValues(const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, int count, uint8* data)
		{
			auto it = mFillValueFunctionMap.find(RTTI_OF(T));
			assert(it != mFillValueFunctionMap.end());

			for (int idx = 0; idx < count; idx++)
				it->second(uniform, referenceUniformA, referenceUniformB, (uint8*)(data + sizeof(T) * idx));
		};
	};


	/**
	 *
	 */
	class ConstantStructBufferFillPolicy : public BaseStructBufferFillPolicy
	{
		RTTI_ENABLE(BaseStructBufferFillPolicy)
	public:
		ConstantStructBufferFillPolicy() = default;

		virtual bool init(utility::ErrorState& errorState);
	};


	//////////////////////////////////////////////////////////////////////////
	// ValueBufferFillPolicy
	//////////////////////////////////////////////////////////////////////////

	/**
	 *
	 */
	class NAPAPI BaseValueBufferFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseValueBufferFillPolicy() = default;
		virtual ~BaseValueBufferFillPolicy() = default;
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
}
