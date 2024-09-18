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
#include "surfacedescriptor.h"

namespace nap
{
	namespace utility
	{
		/**
		 * @return the Vulkan format of the specified surface properties
		 */
		VkFormat NAPAPI getTextureFormat(ESurfaceDataType dataType, ESurfaceChannels channels, EColorSpace colorSpace);

		/**
		 * @return the Vulkan format of the specified surface descriptor
		 */
		VkFormat NAPAPI getTextureFormat(const SurfaceDescriptor& descriptor);

		/**
		 * Transition image to a new layout using an image barrier.
		 */
		void NAPAPI transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint mipLevel, uint mipLevelCount, VkImageAspectFlags aspect);

		/**
		 * Transition image to a new layout using an image barrier.
		 */
		void NAPAPI transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, uint mipLevel, uint mipLevelCount, uint layer, uint layerCount, VkImageAspectFlags aspect);

		/**
		 * Creates mip maps for the specified Vulkan image.
		 */
		void NAPAPI createMipmaps(VkCommandBuffer buffer, VkImage image, VkFormat imageFormat, VkImageLayout targetLayout, VkImageAspectFlags aspect, uint32 texWidth, uint32 texHeight, uint32 mipLevels);

		/**
		 * Creates mip maps for the specified Vulkan image.
		 */
		void NAPAPI createMipmaps(VkCommandBuffer buffer, VkImage image, VkFormat imageFormat, VkImageLayout targetLayout, VkImageAspectFlags aspect, uint32 texWidth, uint32 texHeight, uint32 mipLevels, uint layer, uint layerCount);

		/**
		 * Pushes a full-size blit to the command buffer. Must be called inside a render pass, in onDraw().
		 * Assumes a color texture without mip-maps. The layouts of srcTexture and dstTexture are
		 * transferred to SHADER_READ after the blit operation.
		 * @param commandBuffer the command buffer to push the blit operation to
		 * @param srcTexture the source texture
		 * @param dstTexture the destination texture
		 */
		void NAPAPI blit(VkCommandBuffer commandBuffer, const Texture2D& srcTexture, const Texture2D& dstTexture);
	}
}
