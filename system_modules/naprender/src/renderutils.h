/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>
#include <assert.h>

// Local Includes
#include "imagedata.h"
#include "bufferdata.h"
#include "vk_mem_alloc.h"
#include "surfacedescriptor.h"

namespace nap
{
	/**
	 * Number of rasterization samples
	 */
	enum class ERasterizationSamples : int
	{
		One		= 0x00000001,
		Two		= 0x00000002,
		Four	= 0x00000004,
		Eight	= 0x00000008,
		Sixteen = 0x00000010,
		Max		= 0x00000000		///< Request max available number of rasterization samples.
	};


	namespace utility
	{
		/**
		 * Creates a single or multi-sample renderpass based on rasterization samples and color/depth formats.
		 */
		bool NAPAPI createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState);

		/**
		 * Creates a single or multi-sample depth-only renderpass based depth format.
		 */
		bool NAPAPI createDepthOnlyRenderPass(VkDevice device, VkFormat depthFormat, VkRenderPass& renderPass, utility::ErrorState& errorState);

		/**
		 * Creates a Vulkan image based on the described image usage and given properties.
		 */
		bool NAPAPI create2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState);

		/**
		 * Creates a Vulkan image view based on the described image usage and given properties.
		 */
		bool NAPAPI create2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, VkImageView& outImageView, utility::ErrorState& errorState);

		/**
		 * Creates a Vulkan layered image based on the described image usage and given properties.
		 */
		bool NAPAPI createLayered2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, uint32 layerCount, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImageCreateFlags flags, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState);

		/**
		 * Creates a Vulkan layered image view based on the described image usage and given properties.
		 */
		bool NAPAPI createLayered2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, uint32 layerIndex, uint32 layerCount, VkImageView& outImageView, utility::ErrorState& errorState);

		/**
		 * Creates a Vulkan cube image view based on the described image usage and given properties.
		 */
		bool NAPAPI createCubeImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, uint32 layerCount, VkImageView& outImageView, utility::ErrorState& errorState);

		/**
		 * Destroys a Vulkan image and Vulkan ImageView if present in data
		 */
		void NAPAPI destroyImageAndView(ImageData& data, VkDevice device, VmaAllocator allocator);

		/**
		 * Creates a Vulkan buffer
		 */
		bool NAPAPI createBuffer(VmaAllocator allocator, uint32 size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, BufferData& outBuffer, utility::ErrorState& error);

		/**
		 * Destroys a Vulkan buffer
		 */
		void NAPAPI destroyBuffer(VmaAllocator allocator, BufferData& buffer);

		/**
		 * Uploads data into a staging buffer
		 */
		bool NAPAPI uploadToBuffer(VmaAllocator allocator, uint32 size, const void* data, BufferData& buffer);
	}
}
