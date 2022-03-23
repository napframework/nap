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
#include "fillpolicy.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// BaseFillPolicyEntry
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of FillPolicyEntry.
	 * Binds a struct member name to a fill policy. Used to define StructFillPolicy behavior.
	 */
	class BaseFillPolicyEntry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		BaseFillPolicyEntry() = default;
		virtual ~BaseFillPolicyEntry() = default;
		std::string mName;																	///< Property 'Name': The struct member name of this fill policy entry
	};


	//////////////////////////////////////////////////////////////////////////
	// FillPolicyEntry
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Typed fill policy entry.
	 * Binds a struct member name to a fill policy. Used to define StructFillPolicy behavior.
	 */
	template<typename T>
	class FillPolicyEntry : public BaseFillPolicyEntry
	{
		RTTI_ENABLE(BaseFillPolicyEntry)
	public:
		rtti::ObjectPtr<FillPolicy<T>> mFillPolicy;											///< Property 'FillPolicy': The fill policy associated with the member name
	};


	//////////////////////////////////////////////////////////////////////////
	// StructFillPolicy
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Fill policy for struct buffers.
	 *
	 * Fill policies are initialization utilities that can help fill large blocks of preallocated memory. The fill function
	 * assigns a contiguous block of data based on the specified arguments.
	 *
	 * StructFillPolicy internally uses TypedFillPolicyNumeric mapped to member names to determine how to fill
	 * the struct buffer. The behavior of this fill policy is created in configuration using a set of variable fill policy
	 * entries. Each entry maps a struct member name to a fill policy. The fill() function is then able to fill the buffer
	 * it is bound to automatically.
	 *
	 * Fill policies can be bound to another resource's property (i.e. StructGPUBuffer) in configuration. Typically, an object
	 * will first check if a policy is available, and if so, use it to fill an internal buffer. Any object accepting a fill
	 * policy is free to implement the way fill() is used in their own way however.
	 */
	class NAPAPI StructFillPolicy : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		StructFillPolicy() = default;
		virtual ~StructFillPolicy() = default;

		/**
		 * Fills a preallocated buffer using the specified struct buffer descriptor.
		 * @param numElements the number of elements to fill
		 * @param data pointer to the preallocated data
		 * @param errorState contains the error if the buffer cannot be filled
		 */
		virtual bool fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState);

		std::vector<rtti::ObjectPtr<BaseFillPolicyEntry>> mFillPolicies;					///< Property 'FillPolicies': List of shader variable fill policy entries

	protected:
		/**
		 * Recursively fills the buffer based on the specified uniform struct.
		 * @param uniformStruct the reference uniform used to determine the memory locations to fill
		 * @param data pointer to the preallocated data
		 * @param outElementSize the size of the specified uniform struct in bytes
		 * @param errorState contains the error if the buffer cannot be filled
		 */
		bool fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data, size_t& outElementSize, utility::ErrorState& errorState);

		/**
		 * Returns a fill policy for the given member name if it is specified in the variable fill policies
		 * property, otherwise returns nullptr.
		 * @param name the name of the member to apply the associated fill policy on
		 * @return the buffer fill policy associated with the given member name, otherwise nullptr
		 */
		template<typename T>
		const FillPolicy<T>* findPolicy(const std::string& name)
		{
			for (const auto& fp : mFillPolicies)
			{
				if (fp->mName == name)
				{
					const auto* resolved = rtti_cast<const FillPolicyEntry<T>>(fp.get());
					if (resolved != nullptr)
						return resolved->mFillPolicy.get();

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

	using FillPolicyEntryUInt		= FillPolicyEntry<uint>;
	using FillPolicyEntryInt		= FillPolicyEntry<int>;
	using FillPolicyEntryFloat		= FillPolicyEntry<float>;
	using FillPolicyEntryVec2		= FillPolicyEntry<glm::vec2>;
	using FillPolicyEntryVec3		= FillPolicyEntry<glm::vec3>;
	using FillPolicyEntryVec4		= FillPolicyEntry<glm::vec4>;
	using FillPolicyEntryMat4		= FillPolicyEntry<glm::mat4>;
}
