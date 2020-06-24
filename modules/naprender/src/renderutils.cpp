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
	bool create2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState)
	{
		// Image creation info
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

		// Allocation creation info
		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.flags = 0;
		alloc_info.usage = memoryUsage;

		// Create image using allocator and allocation instructions
		VkResult result = vmaCreateImage(allocator, &image_info, &alloc_info, &outImage, &outAllocation, &outAllocationInfo);
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


	void destroyImageAndView(const ImageData& data, VkDevice device, VmaAllocator allocator)
	{
		if (data.mTextureView != VK_NULL_HANDLE)
			vkDestroyImageView(device, data.mTextureView, nullptr);

		if (data.mTextureImage != VK_NULL_HANDLE)
			vmaDestroyImage(allocator, data.mTextureImage, data.mTextureAllocation);
	}


	bool createBuffer(VmaAllocator allocator, uint32 size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, BufferData& outBuffer, utility::ErrorState& error)
	{
		// Create buffer information 
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Create allocation information
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = 0;

		// Create buffer
		VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &outBuffer.mBuffer, &outBuffer.mAllocation, &outBuffer.mAllocationInfo);
		if (!error.check(result == VK_SUCCESS, "Unable to create buffer"))
			return false;
		return true;
	}


	void destroyBuffer(VmaAllocator allocator, const BufferData& buffer)
	{
		if(buffer.mBuffer != VK_NULL_HANDLE)
			vmaDestroyBuffer(allocator, buffer.mBuffer, buffer.mAllocation);
	}


	bool NAPAPI uploadToBuffer(VmaAllocator allocator, uint32 size, void* data, BufferData& buffer)
	{
		void* mapped_memory;
		if (vmaMapMemory(allocator, buffer.mAllocation, &mapped_memory) != VK_SUCCESS)
			return false;

		memcpy(mapped_memory, data, (size_t)size);
		vmaUnmapMemory(allocator, buffer.mAllocation);
		return true;
	}


	void nap::BufferData::release()
	{
		mAllocation = VK_NULL_HANDLE;
		mBuffer = VK_NULL_HANDLE;
	}
}
