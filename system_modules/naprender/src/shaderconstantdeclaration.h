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
	using ShaderConstantID = uint;
	using ShaderConstantMap = std::map<ShaderConstantID, uint>;
	using ShaderStageConstantMap = std::map<VkShaderStageFlagBits, ShaderConstantMap>;

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
