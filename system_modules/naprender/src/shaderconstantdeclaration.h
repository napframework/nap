/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <vulkan/vulkan_core.h>
#include <string>
#include <map>
#include <utility/dllexport.h>
#include <rtti/typeinfo.h>
#include <nap/numeric.h>


namespace nap
{
	using ShaderConstantID = uint;														// Shader constant ID within a specific shader stage
	using ShaderConstantMap = std::map<ShaderConstantID, uint>;							// Maps constant ID to a constant value
	using ShaderStageConstantMap = std::map<VkShaderStageFlagBits, ShaderConstantMap>;	// Maps shader stage to a constant map

	/**
	 * Stores information of a shader constant declaration. These can be identified in shaders by their special constant ID.
	 * NAP currently only supports unsigned integer constants.
	 *
	 * ~~~~~~~~~~~~~~~{.glsl}
	 * layout (constant_id = 0) const uint QUAD_SAMPLE_COUNT = 8;
	 * ~~~~~~~~~~~~~~~
	 */
	class NAPAPI ShaderConstantDeclaration final
	{
		RTTI_ENABLE()
	public:
		ShaderConstantDeclaration(const std::string& name, uint value, ShaderConstantID id, VkShaderStageFlagBits stage) :
			mName(name), mValue(value), mConstantID(id), mStage(stage) {}

		std::string mName;
		uint mValue;
		ShaderConstantID mConstantID;
		VkShaderStageFlagBits mStage;
	};

	using ShaderConstantDeclarations = std::vector<ShaderConstantDeclaration>;
}
