/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderadvancedutils.h"

namespace nap
{
	namespace utility
	{
		bool createConsumeRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			if (!errorState.check(targetLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "Failed to create render pass. Unsupported target layout."))
				return false;

			bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

			VkAttachmentDescription2 color_attachment = {
				.sType 			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 		= colorFormat,
				.samples 		= samples,
				.loadOp 		= VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp 		= multi_sample ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 	= multi_sample ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : targetLayout
			};

			VkAttachmentDescription2 depth_attachment = {
				.sType 			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 		= depthFormat,
				.samples 		= samples,
				.loadOp 		= VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp 		= VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 	= targetLayout
			};

			VkAttachmentReference2 color_attachment_ref = {
				.sType 			= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 	= 0,
				.layout 		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.aspectMask 	= VK_IMAGE_ASPECT_COLOR_BIT
			};

			VkAttachmentReference2 depth_attachment_ref = {
				.sType 			= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 	= 1,
				.layout 		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				.aspectMask 	= VK_IMAGE_ASPECT_DEPTH_BIT
			};

			VkSubpassDescription2 subpass = {
				.sType						= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
				.flags						= 0,
				.pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount		= 1,
				.pColorAttachments			= &color_attachment_ref,
				.pDepthStencilAttachment	= &depth_attachment_ref
			};

			std::array<VkSubpassDependency2, 2> dependencies;
			dependencies[0] = {
				.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.srcSubpass 		= VK_SUBPASS_EXTERNAL,
				.dstSubpass 		= 0,
				.srcStageMask 		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask 		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask 		= VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask 		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags 	= VK_DEPENDENCY_BY_REGION_BIT,
			};
			dependencies[1] = {
				.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.srcSubpass 		= 0,
				.dstSubpass 		= VK_SUBPASS_EXTERNAL,
				.srcStageMask 		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask 		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask 		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask 		= VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags 	= VK_DEPENDENCY_BY_REGION_BIT
			};

			VkRenderPassCreateInfo2 renderpass_info = {
				.sType 				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
				.subpassCount 		= 1,
				.pSubpasses 		= &subpass,
				.dependencyCount 	= static_cast<uint32_t>(dependencies.size()),
				.pDependencies 		= dependencies.data()
			};

			// Single-sample render pass
			if (!multi_sample)
			{
				std::array<VkAttachmentDescription2, 2> attachments = { color_attachment, depth_attachment };
				renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderpass_info.pAttachments = attachments.data();

				return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
			}

			// Multi-sample render pass
			VkAttachmentDescription2 color_resolve_attachment = {
				.sType				= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 			= colorFormat,
				.samples 			= VK_SAMPLE_COUNT_1_BIT,
				.loadOp 			= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 		= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 		= targetLayout
			};

			VkAttachmentReference2 color_resolve_attachment_ref = {
				.sType 				= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 		= 2,
				.layout 			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT
			};

			VkAttachmentDescription2 depth_resolve_attachment = {
				.sType				= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 			= depthFormat,
				.samples 			= VK_SAMPLE_COUNT_1_BIT,
				.loadOp 			= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 		= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 		= targetLayout
			};

			VkAttachmentReference2 depth_resolve_attachment_ref = {
				.sType 				= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 		= 3,
				.layout				= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				.aspectMask			= VK_IMAGE_ASPECT_DEPTH_BIT
			};

			VkSubpassDescriptionDepthStencilResolve subpass_depth_extension = {
				.sType 				= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE,
				.depthResolveMode 	= VK_RESOLVE_MODE_AVERAGE_BIT,
				.pDepthStencilResolveAttachment = &depth_resolve_attachment_ref
			};
			subpass.pNext = &subpass_depth_extension;
			subpass.pResolveAttachments = &color_resolve_attachment_ref;

			std::array<VkAttachmentDescription2, 4> attachments = { color_attachment, depth_attachment, color_resolve_attachment, depth_resolve_attachment };
			renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderpass_info.pAttachments = attachments.data();

			return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create multi-sample render pass");
		}


		bool createDepthOnlyRenderPass(VkDevice device, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			if (!errorState.check(targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "Failed to create render pass. Unsupported target layout."))
				return false;

			bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

			VkAttachmentDescription2 depth_attachment = {
				.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 		= depthFormat,
				.samples 		= samples,
				.loadOp 		= VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp 		= multi_sample ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 	= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 	= multi_sample ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : targetLayout
			};

			VkAttachmentReference2 depth_attachment_ref = {
				.sType			= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 	= 0,
				.layout 		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				.aspectMask 	= VK_IMAGE_ASPECT_DEPTH_BIT
			};

			VkSubpassDescription2 subpass = {
				.sType						= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
				.pipelineBindPoint 			= VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount 		= 0,
				.pDepthStencilAttachment 	= &depth_attachment_ref
			};

			std::array<VkSubpassDependency2, 2> dependencies;
			dependencies[0] = {
				.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.srcSubpass 		= VK_SUBPASS_EXTERNAL,
				.dstSubpass 		= 0,
				.srcStageMask 		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask 		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask 		= VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask 		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags 	= VK_DEPENDENCY_BY_REGION_BIT
			};
			dependencies[1] = {
				.sType				= VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.srcSubpass 		= 0,
				.dstSubpass 		= VK_SUBPASS_EXTERNAL,
				.srcStageMask 		= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask 		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask 		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask 		= VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags 	= VK_DEPENDENCY_BY_REGION_BIT,
			};

			// Create depth render pass
			VkRenderPassCreateInfo2 renderpass_info = {
				.sType 				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
				.subpassCount 		= 1,
				.pSubpasses 		= &subpass,
				.dependencyCount 	= static_cast<uint32_t>(dependencies.size()),
				.pDependencies 		= dependencies.data()
			};

			// Single-sample render pass
			if (!multi_sample)
			{
				renderpass_info.attachmentCount = 1;
				renderpass_info.pAttachments = &depth_attachment;

				return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
			}

			// Multi-sample render pass
			VkAttachmentDescription2 depth_resolve_attachment = {
				.sType				= VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.format 			= depthFormat,
				.samples 			= VK_SAMPLE_COUNT_1_BIT,
				.loadOp 			= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp 		= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout 		= targetLayout
			};

			VkAttachmentReference2 depth_resolve_attachment_ref = {
				.sType				= VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.attachment 		= 1,
				.layout				= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				.aspectMask 		= VK_IMAGE_ASPECT_DEPTH_BIT
			};

			VkSubpassDescriptionDepthStencilResolve subpass_depth_extension = {
				.sType 				= VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE,
				.depthResolveMode 	= VK_RESOLVE_MODE_AVERAGE_BIT,
				.pDepthStencilResolveAttachment = &depth_resolve_attachment_ref
			};
			subpass.pNext = &subpass_depth_extension;
			subpass.pResolveAttachments = &depth_resolve_attachment_ref;

			std::array<VkAttachmentDescription2, 2> attachments = { depth_attachment, depth_resolve_attachment };
			renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderpass_info.pAttachments = attachments.data();

			return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		}
	}
}
