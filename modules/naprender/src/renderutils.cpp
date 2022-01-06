/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
    bool createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState)
    {
        if (!errorState.check(targetLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "Failed to create render pass. Unsupported target layout."))
            return false;

        bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription color_attachment = {};
        color_attachment.format = colorFormat;
        color_attachment.samples = samples;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = !multi_sample ? targetLayout : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_attachment = {};
        depth_attachment.format = depthFormat;
        depth_attachment.samples = samples;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_info = {};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_info.subpassCount = 1;
        renderpass_info.pSubpasses = &subpass;
        renderpass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderpass_info.pDependencies = dependencies.data();

        // Single-sample render pass
        if (!multi_sample)
        {
            std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
            renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderpass_info.pAttachments = attachments.data();

            return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
        }

        // Multi-sample render pass
        else
        {
            VkAttachmentDescription resolve_attachment{};
            resolve_attachment.format = colorFormat;
            resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            resolve_attachment.finalLayout = targetLayout;

            VkAttachmentReference resolve_attachment_ref{};
            resolve_attachment_ref.attachment = 2;
            resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            subpass.pResolveAttachments = &resolve_attachment_ref;

            std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, resolve_attachment };
            renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderpass_info.pAttachments = attachments.data();

            return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create multi-sample render pass");
        }
    }


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
		alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

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


	bool createBuffer(VmaAllocator allocator, uint32 size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, BufferData& outBuffer, utility::ErrorState& error)
	{
		if (!error.check(size != 0, "Unable to create buffer of size zero"))
			return false;

		// Create buffer information 
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Create allocation information
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = allocationFlags;

		switch (memoryUsage)
		{
		case VMA_MEMORY_USAGE_CPU_TO_GPU:
			allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			break;
		case VMA_MEMORY_USAGE_GPU_TO_CPU:
			allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			break;
		default:
			allocInfo.requiredFlags = 0;
		}

		// Create buffer
		VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &outBuffer.mBuffer, &outBuffer.mAllocation, &outBuffer.mAllocationInfo);
		if (!error.check(result == VK_SUCCESS, "Unable to create buffer, allocation failed"))
			return false;
		return true;
	}


	void destroyBuffer(VmaAllocator allocator, const BufferData& buffer)
	{
		if(buffer.mBuffer != VK_NULL_HANDLE)
			vmaDestroyBuffer(allocator, buffer.mBuffer, buffer.mAllocation);
	}


	bool uploadToBuffer(VmaAllocator allocator, uint32 size, void* data, BufferData& buffer)
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
