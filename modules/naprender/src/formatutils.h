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
#include "shadervariabledeclarations.h"

namespace nap
{
	/**
	 * Returns the vulkan buffer usage flags for a given buffer type.
	 * @param descriptorType the descriptor type to get the associated usage flags of.
	 * @return the vulkan buffer usage flags for a given buffer type.
	 */
	NAPAPI VkBufferUsageFlags getVulkanBufferUsage(EDescriptorType descriptorType);

	/**
	 * Converts nap::EDescriptorType to VkDescriptorType.
	 * @param descriptorType the descriptor type to convert.
	 * @return the vulkan descriptor type for a given buffer object type, asserts if descriptorType
	 * is not 'Uniform' or 'Storage'.
	 */
	NAPAPI VkDescriptorType getVulkanDescriptorType(nap::EDescriptorType descriptorType);

	/**
	* Returns the vulkan format associated with the given numeric rtti type
	* @tparam type the requested element type
	* @return the VkFormat associated with the specified element type, otherwise VK_FORMAT_UNDEFINED
	*/
	NAPAPI VkFormat getVulkanFormat(nap::rtti::TypeInfo type);

	/**
	* Returns the vulkan format associated with ELEMENTTYPE
	* @tparam ELEMENTTYPE the requested element type
	* @return the VkFormat associated with the specified element type, otherwise VK_FORMAT_UNDEFINED
	*/
	template<typename ELEMENTTYPE>
	VkFormat getVulkanFormat()	{ return getVulkanFormat(RTTI_OF(ELEMENTTYPE)); }
}
