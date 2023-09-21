/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "shaderconstantdeclaration.h"

// External Includes
#include <nap/resource.h>
#include <nap/numeric.h>
#include <utility/dllexport.h>


namespace nap
{
	// Shader constant instance
	struct NAPAPI ShaderSpecializationConstantInfo
	{
		std::vector<VkSpecializationMapEntry> mEntries;
		std::vector<uint> mData;
	};


	// Shader constant instance
	class NAPAPI ShaderConstant : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;						///< Property: 'Name' The name of the specialization constant variable in the shader interface
		uint mValue = 0;						///< Property: 'Value' The value to overwrite the default specialization constant variable with
	};


	// Shader constant instance
	class NAPAPI ShaderConstantInstance
	{
	public:
		// Constructor
		ShaderConstantInstance(const ShaderConstantDeclaration& declaration, const ShaderConstant* constant) :
			mDeclaration(declaration), mConstant(constant), mValue((constant != nullptr) ? constant->mValue : declaration.mValue) {}

		/**
		 * @return constant declaration
		 */
		const ShaderConstantDeclaration& getDeclaration() const				{ return mDeclaration; }

		/**
		 * @return constant value
		 */
		const uint& getValue() const										{ return mValue; }

		/**
		 * Sets the constant value. Must be set before pipeline creation for it to be effective.
		 */
		void setValue(uint value)											{ mValue = value; }

		const ShaderConstantDeclaration& mDeclaration;
		const ShaderConstant* mConstant = nullptr;
		uint mValue = 0;
	};
}
