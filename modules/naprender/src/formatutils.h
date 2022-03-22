/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <rtti/typeinfo.h>
#include <nap/numeric.h>
#include <nap/assert.h>
#include <mathutils.h>
#include <vulkan/vulkan_core.h>

// Local includes
#include "basegpubuffer.h"

namespace nap
{
	/**
	* Returns the vulkan format associated with ELEMENTTYPE
	* @tparam ELEMENTTYPE the requested element type
	* @return the VkFormat associated with the specified element type, otherwise VK_FORMAT_UNDEFINED
	*/
	template<typename ELEMENTTYPE>
	static VkFormat getGPUBufferFormat()
	{
		static const std::map<rtti::TypeInfo, VkFormat> format_table =
		{
			{RTTI_OF(uint),			VkFormat::VK_FORMAT_R32_UINT},
			{RTTI_OF(int),			VkFormat::VK_FORMAT_R32_SINT},
			{RTTI_OF(float),		VkFormat::VK_FORMAT_R32_SFLOAT},
			{RTTI_OF(glm::vec2),	VkFormat::VK_FORMAT_R32G32_SFLOAT},
			{RTTI_OF(glm::vec3),	VkFormat::VK_FORMAT_R32G32B32_SFLOAT},
			{RTTI_OF(glm::vec4),	VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT}
		};

		const auto it = format_table.find(RTTI_OF(ELEMENTTYPE));
		if (it != format_table.end())
			return it->second;

		NAP_ASSERT_MSG(false, "Unsupported vertex buffer type");
		return VkFormat::VK_FORMAT_UNDEFINED;
	}


	/**
	 * Returns the vulkan buffer usage flags for a given buffer type.
	 * @param descriptorType the descriptor type to get the associated usage flags of.
	 * @return the vulkan buffer usage flags for a given buffer type.
	 */
	static VkBufferUsageFlags getBufferUsage(EDescriptorType descriptorType)
	{
		if (descriptorType == EDescriptorType::Uniform)
			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		else if (descriptorType == EDescriptorType::Storage)
			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		return 0;
	}


	/**
	 * Converts nap::EDescriptorType to VkDescriptorType.
	 * @param descriptorType the descriptor type to convert.
	 * @return the vulkan descriptor type for a given buffer object type, asserts if descriptorType
	 * is not 'Uniform' or 'Storage'.
	 */
	static VkDescriptorType getVulkanDescriptorType(nap::EDescriptorType descriptorType)
	{
		if (descriptorType == nap::EDescriptorType::Uniform)
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		else if (descriptorType == nap::EDescriptorType::Storage)
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		NAP_ASSERT_MSG(descriptorType != nap::EDescriptorType::Default, "nap::DescriptorType 'Default' cannot be used as a Vulkan descriptor type");
		NAP_ASSERT_MSG(false, "Unsupported descriptor type");

		return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}
