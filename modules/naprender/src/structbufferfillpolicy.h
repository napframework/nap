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
#include "valuebufferfillpolicy.h"

namespace nap
{
	// Forward declares
	class UniformValue;

	//////////////////////////////////////////////////////////////////////////
	// ShaderVariableFillPolicyEntry
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of TypedShaderVariableFillPolicyEntry<T> 
	 * Binds a shader variable name to a value fill policy
	 */
	class BaseShaderVariableFillPolicyEntry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseShaderVariableFillPolicyEntry() = default;
		virtual ~BaseShaderVariableFillPolicyEntry() = default;
		std::string mName;																			///< Property 'Name':
	};

	/**
	 * Binds a shader variable name to a value fill policy
	 */
	template<typename T>
	class TypedShaderVariableFillPolicyEntry : public BaseShaderVariableFillPolicyEntry
	{
		RTTI_ENABLE(BaseShaderVariableFillPolicyEntry)
	public:
		rtti::ObjectPtr<TypedValueBufferFillPolicy<T>> mValueFillPolicy;							///< Property 'ValueFillPolicy':
	};


	//////////////////////////////////////////////////////////////////////////
	// StructBufferFillPolicy
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 
	 */
	class NAPAPI StructBufferFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		StructBufferFillPolicy() = default;
		virtual ~StructBufferFillPolicy() = default;

		/**
		 * Fills an allocated buffer using the specified struct buffer descriptor
		 */
		virtual bool fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState);

		std::vector<rtti::ObjectPtr<BaseShaderVariableFillPolicyEntry>> mVariableFillPolicies;		///< Property 'VariableFillPolicies':

	private:
		/**
		 * 
		 */
		bool fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data, size_t& outElementSize, utility::ErrorState& errorState);

		/**
		 * 
		 */
		template<typename T>
		const TypedValueBufferFillPolicy<T>* findPolicy(const std::string& name)
		{
			for (const auto& fp : mVariableFillPolicies)
			{
				if (fp->mName == name)
				{
					const TypedValueBufferFillPolicy<T>* resolved = rtti_cast<TypedValueBufferFillPolicy<T>>(fp.get());
					if (resolved != nullptr)
						return resolved;

					// Names match but types do not
					assert(false); 
					return nullptr;
				}
			}
			return nullptr;
		}
	};


	//////////////////////////////////////////////////////////////////////////
	// TypedShaderVariableFillPolicyEntry type definitions
	//////////////////////////////////////////////////////////////////////////

	using UIntFillPolicyEntry = TypedShaderVariableFillPolicyEntry<uint>;
	using IntFillPolicyEntry = TypedShaderVariableFillPolicyEntry<int>;
	using FloatFillPolicyEntry = TypedShaderVariableFillPolicyEntry<float>;
	using Vec2FillPolicyEntry = TypedShaderVariableFillPolicyEntry<glm::vec2>;
	using Vec3FillPolicyEntry = TypedShaderVariableFillPolicyEntry<glm::vec3>;
	using Vec4FillPolicyEntry = TypedShaderVariableFillPolicyEntry<glm::vec4>;
	using Mat4FillPolicyEntry = TypedShaderVariableFillPolicyEntry<glm::mat4>;
}
