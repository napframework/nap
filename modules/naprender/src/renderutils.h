#pragma once

// External Includes
#include <vulkan/vulkan_core.h>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>

// Local Includes
#include "vk_mem_alloc.h"

namespace nap
{
	/**
	 * Vulkan Image Structure
	 */
	struct NAPAPI ImageData
	{
		// Default Constructor
		ImageData() = default;

		VkImage				mTextureImage = nullptr;						///< Vulkan Image
		VkImageView			mTextureView = nullptr;							///< Vulkan Image view
		VmaAllocation		mTextureAllocation = nullptr;					///< Vulkan single memory allocation
		VmaAllocationInfo	mTextureAllocationInfo;							///< Vulkan memory allocation information
		VkImageLayout		mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;		///< Vulkan image layout
	};

	/**
	 * Creates a Vulkan image based on the described image usage and given properties.
	 */
	bool NAPAPI create2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, const VmaAllocationCreateInfo& allocationUsage, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState);

	/**
	 * Creates a Vulkan image view based.
	 */
	bool NAPAPI create2DImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& outImageView, utility::ErrorState& errorState);

	/**
	 * Destroys a Vulkan image and Vulkan ImageView if present in data
	 */
	void NAPAPI destroyImageAndView(ImageData& data, VkDevice device, VmaAllocator allocator);
}
