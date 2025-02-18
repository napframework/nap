/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	namespace utility
	{
        bool createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, bool clear, VkRenderPass& renderPass, utility::ErrorState& errorState)
        {
            if (!errorState.check(targetLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "Failed to create render pass. Unsupported target layout."))
                return false;

            bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

            VkAttachmentDescription color_attachment = {
				.format 		= colorFormat,
				.samples 		= samples,
				.loadOp 		= clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp 		= multi_sample ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 	= clear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.finalLayout 	= multi_sample ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : targetLayout
			};

            VkAttachmentDescription depth_attachment = {
				.format = depthFormat,
				.samples = samples,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

            VkAttachmentReference color_attachment_ref = {
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

            VkAttachmentReference depth_attachment_ref = {
            	.attachment = 1,
            	.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

            VkSubpassDescription subpass = {
           		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
           		.colorAttachmentCount = 1,
           		.pColorAttachments = &color_attachment_ref,
           		.pDepthStencilAttachment = &depth_attachment_ref
			};

            std::array<VkSubpassDependency, 2> dependencies;
            dependencies[0] = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
            	.dstSubpass = 0,
            	.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            	.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            	.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
            	.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            	.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			};
            dependencies[1] = {
				.srcSubpass = 0,
           		.dstSubpass = VK_SUBPASS_EXTERNAL,
           		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
           		.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
           		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
           		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
           		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			};

            VkRenderPassCreateInfo renderpass_info = {
            	.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            	.subpassCount = 1,
            	.pSubpasses = &subpass,
            	.dependencyCount = static_cast<uint32_t>(dependencies.size()),
            	.pDependencies = dependencies.data()
			};

            // Single-sample render pass
            if (!multi_sample)
            {
                std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
                renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderpass_info.pAttachments = attachments.data();

                return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
            }

			// Multi-sample render pass
			VkAttachmentDescription resolve_attachment = {
				.format = colorFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = targetLayout
			};

			VkAttachmentReference resolve_attachment_ref = {
				.attachment = 2,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			subpass.pResolveAttachments = &resolve_attachment_ref;

			std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, resolve_attachment };
			renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderpass_info.pAttachments = attachments.data();

			return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create multi-sample render pass");
        }


		bool create2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState)
		{
			// Image creation info
			VkImageCreateInfo image_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent = {
					.width = width,
					.height = height,
					.depth = 1,
				},
				.mipLevels = mipLevels,
				.arrayLayers = 1,
				.samples = samples,
				.tiling = tiling,
				.usage = imageUsage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			// Allocation creation info
			VmaAllocationCreateInfo alloc_info = {
				.flags = 0,
				.usage = memoryUsage,
				.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			};

			// Create image using allocator and allocation instructions
			VkResult result = vmaCreateImage(allocator, &image_info, &alloc_info, &outImage, &outAllocation, &outAllocationInfo);
			return errorState.check(result == VK_SUCCESS, "Failed to create image for texture");
		}


		bool create2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, VkImageView& outImageView, utility::ErrorState& errorState)
		{
			VkImageViewCreateInfo view_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = format,
				.subresourceRange = {
					.aspectMask = aspectFlags,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			return errorState.check(vkCreateImageView(device, &view_info, nullptr, &outImageView) == VK_SUCCESS, "Failed to create image view");
		}


		bool createLayered2DImage(VmaAllocator allocator, uint32 width, uint32 height, VkFormat format, uint32 mipLevels, uint32 layerCount, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImageCreateFlags flags, VkImage& outImage, VmaAllocation& outAllocation, VmaAllocationInfo& outAllocationInfo, utility::ErrorState& errorState)
		{
			// Image creation info
			VkImageCreateInfo image_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.flags = flags,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent = {
					.width = width,
					.height = height,
					.depth = 1
				},
				.mipLevels = mipLevels,
				.arrayLayers = layerCount,
				.samples = samples,
				.tiling = tiling,
				.usage = imageUsage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			// Allocation creation info
			VmaAllocationCreateInfo alloc_info = {
				.flags = 0,
				.usage = memoryUsage,
				.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			};

			// Create image using allocator and allocation instructions
			VkResult result = vmaCreateImage(allocator, &image_info, &alloc_info, &outImage, &outAllocation, &outAllocationInfo);
			return errorState.check(result == VK_SUCCESS, "Failed to create image for texture");
		}


		bool createLayered2DImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, uint32 layerIndex, uint32 layerCount, VkImageView& outImageView, utility::ErrorState& errorState)
		{
			VkImageViewCreateInfo view_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = format,
				.subresourceRange = {
					.aspectMask = aspectFlags,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = layerIndex,
					.layerCount = layerCount
				}
			};
			return errorState.check(vkCreateImageView(device, &view_info, nullptr, &outImageView) == VK_SUCCESS, "Failed to create image view");
		}


		bool createCubeImageView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevels, VkImageAspectFlags aspectFlags, uint32 layerCount, VkImageView& outImageView, utility::ErrorState& errorState)
		{
			VkImageViewCreateInfo view_info = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
				.format = format,
				.subresourceRange = {
					.aspectMask = aspectFlags,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = 0,
					.layerCount = layerCount
				}
			};
			return errorState.check(vkCreateImageView(device, &view_info, nullptr, &outImageView) == VK_SUCCESS, "Failed to create image view");
		}


		void destroyImageAndView(ImageData& data, VkDevice device, VmaAllocator allocator)
		{
			// Subviews
			for (uint i = 0; i < data.getSubViewCount(); i++)
			{
				if (data.getSubView(i) != VK_NULL_HANDLE)
				{
					vkDestroyImageView(device, data.mSubViews[i], nullptr);
					data.mSubViews[i] = VK_NULL_HANDLE;
				}
			}

			// View
			if (data.mView != VK_NULL_HANDLE)
			{
				vkDestroyImageView(device, data.mView, nullptr);
				data.mView = VK_NULL_HANDLE;
			}

			// Image
			if (data.mImage != VK_NULL_HANDLE)
			{
				vmaDestroyImage(allocator, data.mImage, data.mAllocation);
				data.mImage = VK_NULL_HANDLE;
				data.mAllocation = VK_NULL_HANDLE;
			}
		}


		bool createBuffer(VmaAllocator allocator, uint32 size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, BufferData& outBuffer, utility::ErrorState& errorState)
		{
			if (!errorState.check(size != 0, "Unable to create buffer of size zero"))
				return false;

			// Create buffer information 
			VkBufferCreateInfo buffer_info = {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = size,
				.usage = bufferUsage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			// Create allocation information
			VmaAllocationCreateInfo alloc_info = {
				.flags = allocationFlags,
				.usage = memoryUsage
			};

			switch (memoryUsage)
			{
				case VMA_MEMORY_USAGE_CPU_TO_GPU:
					alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
					break;
				case VMA_MEMORY_USAGE_GPU_TO_CPU:
				default:
					alloc_info.requiredFlags = 0;
			}

			// Create buffer
			VkResult result = vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &outBuffer.mBuffer, &outBuffer.mAllocation, &outBuffer.mAllocationInfo);
			if (!errorState.check(result == VK_SUCCESS, "Unable to create buffer, allocation failed"))
				return false;

			outBuffer.mUsage = bufferUsage;
			return true;
		}


		void destroyBuffer(VmaAllocator allocator, BufferData& buffer)
		{
			if (buffer.mBuffer != VK_NULL_HANDLE)
			{
				vmaDestroyBuffer(allocator, buffer.mBuffer, buffer.mAllocation);
				buffer.mBuffer = VK_NULL_HANDLE;
			}
		}


		bool uploadToBuffer(VmaAllocator allocator, uint32 size, const void* data, BufferData& buffer)
		{
			void* mapped_memory = nullptr;
			if (vmaMapMemory(allocator, buffer.mAllocation, &mapped_memory) != VK_SUCCESS)
				return false;

			std::memcpy(mapped_memory, data, static_cast<size_t>(size));
			vmaUnmapMemory(allocator, buffer.mAllocation);
			return true;
		}
	}
}
