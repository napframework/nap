/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderadvancedutils.h"

// External Includes
#include <vk_mem_alloc.h>

namespace nap
{
	namespace utility
	{
		bool createConsumeRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkImageLayout targetLayout, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			if (!errorState.check(targetLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || targetLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "Failed to create render pass. Unsupported target layout."))
				return false;

			bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

			VkAttachmentDescription2 color_attachment = {};
			color_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			color_attachment.format = colorFormat;
			color_attachment.samples = samples;
			color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachment.storeOp = multi_sample ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
			color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_attachment.finalLayout = multi_sample ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : targetLayout;

			VkAttachmentDescription2 depth_attachment = {};
			depth_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			depth_attachment.format = depthFormat;
			depth_attachment.samples = samples;
			depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_attachment.finalLayout = targetLayout;

			VkAttachmentReference2 color_attachment_ref = {};
			color_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			color_attachment_ref.attachment = 0;
			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			color_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			VkAttachmentReference2 depth_attachment_ref = {};
			depth_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depth_attachment_ref.attachment = 1;
			depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_attachment_ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkSubpassDescription2 subpass = {};
			subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpass.flags = 0;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_ref;
			subpass.pDepthStencilAttachment = &depth_attachment_ref;

			std::array<VkSubpassDependency2, 2> dependencies = {};
			dependencies[0] = {};
			dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1] = {};
			dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo2 renderpass_info = {};
			renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
			renderpass_info.subpassCount = 1;
			renderpass_info.pSubpasses = &subpass;
			renderpass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderpass_info.pDependencies = dependencies.data();

			// Single-sample render pass
			if (!multi_sample)
			{
				std::array<VkAttachmentDescription2, 2> attachments = { color_attachment, depth_attachment };
				renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderpass_info.pAttachments = attachments.data();

				return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
			}

			// Multi-sample render pass
			VkAttachmentDescription2 color_resolve_attachment = {};
			color_resolve_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			color_resolve_attachment.format = colorFormat;
			color_resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			color_resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color_resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_resolve_attachment.finalLayout = targetLayout;

			VkAttachmentReference2 color_resolve_attachment_ref = {};
			color_resolve_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			color_resolve_attachment_ref.attachment = 2;
			color_resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			color_resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			VkAttachmentDescription2 depth_resolve_attachment = {};
			depth_resolve_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			depth_resolve_attachment.format = depthFormat;
			depth_resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depth_resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depth_resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_resolve_attachment.finalLayout = targetLayout;

			VkAttachmentReference2 depth_resolve_attachment_ref = {};
			depth_resolve_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depth_resolve_attachment_ref.attachment = 3;
			depth_resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depth_resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkSubpassDescriptionDepthStencilResolve subpass_depth_extension = {};
			subpass_depth_extension.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
			subpass_depth_extension.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			subpass_depth_extension.pDepthStencilResolveAttachment = &depth_resolve_attachment_ref;
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

			VkAttachmentDescription2 depth_attachment = {};
			depth_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			depth_attachment.format = depthFormat;
			depth_attachment.samples = samples;
			depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_attachment.storeOp = multi_sample ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
			depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_attachment.finalLayout = multi_sample ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : targetLayout;

			VkAttachmentReference2 depth_attachment_ref = {};
			depth_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depth_attachment_ref.attachment = 0;
			depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depth_attachment_ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkSubpassDescription2 subpass = {};
			subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 0;
			subpass.pDepthStencilAttachment = &depth_attachment_ref;

			std::array<VkSubpassDependency2, 2> dependencies;
			dependencies[0] = {};
			dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1] = {};
			dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo2 renderpass_info = {};
			renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
			renderpass_info.subpassCount = 1;
			renderpass_info.pSubpasses = &subpass;
			renderpass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderpass_info.pDependencies = dependencies.data();

			if (!multi_sample)
			{
				renderpass_info.attachmentCount = 1;
				renderpass_info.pAttachments = &depth_attachment;

				return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
			}

			VkAttachmentDescription2 depth_resolve_attachment = {};
			depth_resolve_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
			depth_resolve_attachment.format = depthFormat;
			depth_resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depth_resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depth_resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_resolve_attachment.finalLayout = targetLayout;

			VkAttachmentReference2 depth_resolve_attachment_ref = {};
			depth_resolve_attachment_ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			depth_resolve_attachment_ref.attachment = 1;
			depth_resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			depth_resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkSubpassDescriptionDepthStencilResolve subpass_depth_extension = {};
			subpass_depth_extension.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
			subpass_depth_extension.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			subpass_depth_extension.pDepthStencilResolveAttachment = &depth_resolve_attachment_ref;
			subpass.pNext = &subpass_depth_extension;
			subpass.pResolveAttachments = &depth_resolve_attachment_ref;

			std::array<VkAttachmentDescription2, 2> attachments = { depth_attachment, depth_resolve_attachment };
			renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderpass_info.pAttachments = attachments.data();

			return errorState.check(vkCreateRenderPass2(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		}
	}
}
