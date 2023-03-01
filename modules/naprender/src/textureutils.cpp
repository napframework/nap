/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <renderservice.h>

// Local Includes
#include "textureutils.h"

namespace nap
{
	VkFormat getTextureFormat(ESurfaceDataType dataType, ESurfaceChannels channels, EColorSpace colorSpace)
	{
		switch (channels)
		{
		case ESurfaceChannels::R:
		{
			switch (dataType)
			{
			case ESurfaceDataType::BYTE:
				return colorSpace == EColorSpace::Linear ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_SRGB;
			case ESurfaceDataType::FLOAT:
				return VK_FORMAT_R32_SFLOAT;
			case ESurfaceDataType::USHORT:
				return VK_FORMAT_R16_UNORM;
			}
			break;
		}
		case ESurfaceChannels::RGBA:
		{
			switch (dataType)
			{
			case ESurfaceDataType::BYTE:
				return colorSpace == EColorSpace::Linear ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
			case ESurfaceDataType::FLOAT:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ESurfaceDataType::USHORT:
				return VK_FORMAT_R16G16B16A16_UNORM;
			}
			break;
		}
		case ESurfaceChannels::BGRA:
		{
			switch (dataType)
			{
			case ESurfaceDataType::BYTE:
				return colorSpace == EColorSpace::Linear ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8A8_SRGB;
			case ESurfaceDataType::FLOAT:
				return VK_FORMAT_UNDEFINED;
			case ESurfaceDataType::USHORT:
				return VK_FORMAT_UNDEFINED;
			}
			break;
		}
		case ESurfaceChannels::D:
		{
			switch (dataType)
			{
			case ESurfaceDataType::FLOAT:
				return VK_FORMAT_D32_SFLOAT;
			case ESurfaceDataType::USHORT:
				return VK_FORMAT_D16_UNORM;
			}
			break;
		}
		NAP_ASSERT_MSG(false, "Surface descriptor could not be resolved to valid/supported texture format");
		}
		return VK_FORMAT_UNDEFINED;
	}


	VkFormat getTextureFormat(const SurfaceDescriptor& descriptor)
	{
		return getTextureFormat(descriptor.getDataType(), descriptor.getChannels(), descriptor.getColorSpace());
	}


	void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
		uint mipLevel, uint mipLevelCount, VkImageAspectFlags aspect)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspect;
		barrier.subresourceRange.baseMipLevel = mipLevel;
		barrier.subresourceRange.levelCount = mipLevelCount;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}


	void blit(VkCommandBuffer commandBuffer, const Texture2D& srcTexture, const Texture2D& dstTexture)
	{
		// Transition to transfer src
		VkImageLayout src_tex_layout = srcTexture.getHandle().getLayout();
		if (src_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			transitionImageLayout(commandBuffer, srcTexture.getHandle().getImage(),
				src_tex_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		// Transition to transfer dst
		VkImageLayout dst_tex_layout = dstTexture.getHandle().getLayout();
		if (dst_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			transitionImageLayout(commandBuffer, dstTexture.getHandle().getImage(),
				dst_tex_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		// Create blit structure
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { srcTexture.getWidth(), srcTexture.getHeight(), 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { dstTexture.getWidth(), dstTexture.getHeight(), 1 };

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Blit to output
		vkCmdBlitImage(commandBuffer,
			srcTexture.getHandle().getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstTexture.getHandle().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		// Transition to shader read
		transitionImageLayout(commandBuffer, srcTexture.getHandle().getImage(),
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 1, VK_IMAGE_ASPECT_COLOR_BIT);

		// Transition to shader read
		transitionImageLayout(commandBuffer, dstTexture.getHandle().getImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}
