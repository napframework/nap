/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>

// Local Includes
#include "vk_mem_alloc.h"
#include "surfacedescriptor.h"

namespace nap
{
	/**
	 * Vulkan Image Data Structure.
	 * Binds image data, view and memory allocation information together for easy usage.
	 */
	struct NAPAPI ImageData
	{
		// Default Constructor
		ImageData() = default;

		/**
		 * @return Handle to Vulkan Image View
		 */
		VkImageView getView() const			{ return mView; }

		/**
		 * @return Handle to Vulkan Image Data
		 */
		VkImage getImage() const			{ return mImage; }

		/**
		 * @return Current Vulkan image layout.
		 */
		VkImageLayout getLayout() const		{ return mCurrentLayout; }

		/**
		 * Releases the image and view, resetting all the handles to null. Does not delete it.
		 */
		void release();

		VkImage				mImage = VK_NULL_HANDLE;						///< Vulkan Image
		VkImageView			mView = VK_NULL_HANDLE;							///< Vulkan Image view
		VmaAllocation		mAllocation = VK_NULL_HANDLE;					///< Vulkan single memory allocation
		VmaAllocationInfo	mAllocationInfo;								///< Vulkan memory allocation information
		VkImageLayout		mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;		///< Vulkan image layout
	};


	/**
	 * Vulkan Buffer Data Structure
	 * Binds a buffer, usage information, memory allocation and allocation information together.
	 */
	struct NAPAPI BufferData
	{
		// Default constructor
		BufferData() = default;

		/**
		 * Releases the buffer, resetting all the handles to null. Does not delete it.
		 */
		void					release();

		VmaAllocation			mAllocation = VK_NULL_HANDLE;				///< Vulkan memory allocation handle
		VmaAllocationInfo		mAllocationInfo;							///< Vulkan allocation information
		VkBufferUsageFlags		mUsage = 0;									///< Usage flags
		VkBuffer				mBuffer = VK_NULL_HANDLE;					///< Vulkan buffer
	};


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
	 * Creates a Vulkan image view based.
	 */
	bool NAPAPI create2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, VkImageView& outImageView, utility::ErrorState& errorState);

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
