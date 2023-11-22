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

	/**
	 * Shader constant resource. Assigns an unsigned integer constant to a material.
	 *
	 * Shaders are parsed on initialization, but not actually compiled until creation of a graphics pipeline. Shader
	 * constants implement Vulkan specialization constants and are a powerful way to compile specialized versions of
	 * shaders per material instance. Graphics and compute pipeline keys are distinguished by a shader constant
	 * hash that is generated from the material instance resource. This way you can benefit from compiler optimizations
	 * like loop unrolling and branch elimination per material setup.
	 * 
	 * ~~~~~{.frag}
	 * layout (constant_id = 0) const uint SAMPLE_COUNT = 8;
	 * layout (constant_id = 1) const uint USE_EXPENSIVE_ROUTINE = 0;
	 *
	 * // main()
	 * // Loop unrolling
	 * for (uint i = 0; i < SAMPLE_COUNT; i++)
	 * {
	 *		// Perform expensive texture samples here
	 * }
	 *
	 * // Branch elimination
	 * if (USE_EXPENSIVE_ROUTINE > 0)
	 * {
	 *		// Perform an expensive computation here
	 * }
	 * else
	 * {
	 *		// Exit early here
	 * }
	 * ~~~~~
	 */
	class NAPAPI ShaderConstant : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mName;						///< Property: 'Name' The name of the specialization constant variable in the shader interface
		uint mValue = 0;						///< Property: 'Value' The value to overwrite the default specialization constant variable with
	};


	/**
	 * Shader constant instance. Assigns a unsigned integer constant to a material instance.
	 * 
	 * Shaders are parsed on initialization, but not actually compiled until creation of a graphics pipeline. Shader
	 * constants implement Vulkan specialization constants and are a powerful way to compile specialized versions of
	 * shaders per material instance. Graphics and compute pipeline keys are distinguished by a shader constant
	 * hash that is generated from the material instance resource. This way you can benefit from compiler optimizations
	 * like loop unrolling and branch elimination per material setup.
	 *
	 * ~~~~~{.frag}
	 * layout (constant_id = 0) const uint SAMPLE_COUNT = 8;
	 * layout (constant_id = 1) const uint USE_EXPENSIVE_ROUTINE = 0;
	 *
	 * // main()
	 * // Loop unrolling
	 * for (uint i = 0; i < SAMPLE_COUNT; i++)
	 * {
	 *		// Perform expensive texture samples here
	 * }
	 *
	 * // Branch elimination
	 * if (USE_EXPENSIVE_ROUTINE > 0)
	 * {
	 *		// Perform an expensive computation here
	 * }
	 * else
	 * {
	 *		// Exit early here
	 * }
	 * ~~~~~
	 */
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
		const uint getValue() const											{ return mValue; }

		const ShaderConstantDeclaration& mDeclaration;
		const ShaderConstant* mConstant = nullptr;
		uint mValue = 0;
	};
}
