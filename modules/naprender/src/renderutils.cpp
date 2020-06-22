#pragma once

// External Includes
#include <renderservice.h>

// Local Includes
#include "renderutils.h"

RTTI_BEGIN_ENUM(nap::ERasterizationSamples)
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::One,		"One"),
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::Two,		"Two"),
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::Four,		"Four"),
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::Eight,		"Eight"),
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::Sixteen,	"Sixteen"),
	RTTI_ENUM_VALUE(nap::ERasterizationSamples::Max,		"Max")
RTTI_END_ENUM;

namespace nap
{
	bool create2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, const VmaAllocationCreateInfo& allocationUsage, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState)
	{
		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = mipLevels;
		image_info.arrayLayers = 1;
		image_info.format = format;
		image_info.tiling = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = imageUsage;
		image_info.samples = samples;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vmaCreateImage(allocator, &image_info, &allocationUsage, &outImage, &outAllocation, &outAllocationInfo);
		if (!errorState.check(result == VK_SUCCESS, "Failed to create image for texture"))
			return false;

		return true;
	}


	bool create2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, VkImageView& outImageView, utility::ErrorState& errorState)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (!errorState.check(vkCreateImageView(device, &viewInfo, nullptr, &outImageView) == VK_SUCCESS, "Failed to create image view"))
			return false;

		return true;
	}


	void NAPAPI destroyImageAndView(const ImageData& data, VkDevice device, VmaAllocator allocator)
	{
		if (data.mTextureView != nullptr)
			vkDestroyImageView(device, data.mTextureView, nullptr);

		if (data.mTextureImage != nullptr)
			vmaDestroyImage(allocator, data.mTextureImage, data.mTextureAllocation);
	}
}