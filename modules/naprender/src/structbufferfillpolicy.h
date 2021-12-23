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
}
