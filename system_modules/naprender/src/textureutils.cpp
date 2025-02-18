/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <renderservice.h>

// Local Includes
#include "textureutils.h"
#include "texture.h"

namespace nap
{
	namespace utility
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


        static void transitionImageLayoutInternal(VkCommandBuffer commandBuffer, VkImage image,
            VkImageLayout oldLayout, VkImageLayout newLayout,
            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
            VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
            uint mipLevel, uint mipLevelCount,
            uint layer, uint layerCount, VkImageAspectFlags aspect)
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
            barrier.subresourceRange.baseArrayLayer = layer;
            barrier.subresourceRange.layerCount = layerCount;
            barrier.srcAccessMask = srcAccessMask;
            barrier.dstAccessMask = dstAccessMask;
            vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }


		void transitionImageLayout(VkCommandBuffer commandBuffer,
            ImageData& image, VkImageLayout newLayout,
			VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
			VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			uint mipLevel, uint mipLevelCount, VkImageAspectFlags aspect)
		{
			transitionImageLayoutInternal(commandBuffer, image.getImage(), image.getLayout(), newLayout, srcAccessMask, dstAccessMask, srcStage, dstStage, mipLevel, mipLevelCount, 0, VK_REMAINING_ARRAY_LAYERS, aspect);

            // Update image layout
            image.mCurrentLayout = newLayout;
		}


		void transitionImageLayout(VkCommandBuffer commandBuffer,
            ImageData& image, VkImageLayout newLayout,
			VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
			VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			uint mipLevel, uint mipLevelCount,
			uint layer, uint layerCount, VkImageAspectFlags aspect)
		{
            transitionImageLayoutInternal(commandBuffer, image.getImage(), image.getLayout(), newLayout, srcAccessMask, dstAccessMask, srcStage, dstStage, mipLevel, mipLevelCount, layer, layerCount, aspect);

            // Update image layout
            image.mCurrentLayout = newLayout;
		}


		int computeMipLevel(const SurfaceDescriptor& descriptor)
		{
			return static_cast<int>(std::floor(std::log2(std::max(descriptor.getWidth(), descriptor.getHeight())))) + 1;
		}


		void createMipmaps(VkCommandBuffer buffer, ImageData& image, VkFormat imageFormat, VkImageLayout targetLayout, VkImageAspectFlags aspect, uint32 texWidth, uint32 texHeight, uint32 mipLevels)
		{
			return createMipmaps(buffer, image, imageFormat, targetLayout, aspect, texWidth, texHeight, mipLevels, 0, 1);
		}


		void createMipmaps(VkCommandBuffer buffer, ImageData& image, VkFormat imageFormat, VkImageLayout targetLayout, VkImageAspectFlags aspect, uint32 texWidth, uint32 texHeight, uint32 mipLevels, uint layer, uint layerCount)
		{
			auto mip_width = static_cast<int32>(texWidth);
			auto mip_height = static_cast<int32>(texHeight);

			for (uint32 i = 1; i < mipLevels; i++)
			{
				// Prepare src LOD for blit operation
                transitionImageLayoutInternal(buffer, image.getImage(),
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					i-1, 1, layer, layerCount,
					aspect);

				// Create blit structure
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = {mip_width, mip_height, 1 };
				blit.srcSubresource.aspectMask = aspect;
				blit.srcSubresource.mipLevel = i-1;
				blit.srcSubresource.baseArrayLayer = layer;
				blit.srcSubresource.layerCount = layerCount;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = aspect;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = layer;
				blit.dstSubresource.layerCount = layerCount;

				// Blit
				vkCmdBlitImage(buffer,
					image.getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					image.getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &blit,
					VK_FILTER_LINEAR);

				// Prepare src LOD for shader read
				transitionImageLayoutInternal(buffer, image.getImage(),
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, targetLayout,
					VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					i-1, 1, layer, layerCount,
					aspect);

				if (mip_width > 1) mip_width /= 2;
				if (mip_height > 1) mip_height /= 2;
			}

			// Prepare final LOD for shader read
            transitionImageLayoutInternal(buffer, image.getImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetLayout,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				mipLevels-1, 1, layer, layerCount,
				aspect);
		}


		void blit(VkCommandBuffer commandBuffer, Texture2D& srcTexture, Texture2D& dstTexture)
		{
			// Transition to transfer src
			const auto src_tex_layout = srcTexture.getHandle().getLayout();
			if (src_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				transitionImageLayout(commandBuffer,
                    srcTexture.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			// Transition to transfer dst
			const auto dst_tex_layout = dstTexture.getHandle().getLayout();
			if (dst_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				transitionImageLayout(commandBuffer,
					dstTexture.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			// Create blit structure
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { srcTexture.getWidth(), srcTexture.getHeight(), 1 };
			blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
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

            // Restore initial layout
			transitionImageLayout(commandBuffer,
                srcTexture.getHandle(), src_tex_layout,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);

            // Restore initial layout
			transitionImageLayout(commandBuffer,
                dstTexture.getHandle(), dst_tex_layout,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		}


		void copy(VkCommandBuffer commandBuffer, Texture2D& srcTexture, Texture2D& dstTexture)
		{
			assert(srcTexture.getSize() == dstTexture.getSize());
			assert(srcTexture.getFormat() == dstTexture.getFormat());

			// Transition to transfer src
			const auto src_tex_layout = srcTexture.getHandle().getLayout();
			if (src_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				transitionImageLayout(commandBuffer,
                    srcTexture.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			// Transition to transfer dst
			const auto dst_tex_layout = dstTexture.getHandle().getLayout();
			if (dst_tex_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				transitionImageLayout(commandBuffer,
                    dstTexture.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			// Create blit structure
			VkImageCopy region = {};
			region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			region.srcOffset = { 0, 0, 0 };
			region.dstOffset = { 0, 0, 0 };
			region.extent = { static_cast<uint>(srcTexture.getWidth()), static_cast<uint>(srcTexture.getHeight()), 1 };

			// Blit to output
			vkCmdCopyImage(commandBuffer,
				srcTexture.getHandle().getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstTexture.getHandle().getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &region);

            // Restore initial layout
			transitionImageLayout(commandBuffer,
                srcTexture.getHandle(), src_tex_layout,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);

            // Restore initial layout
			transitionImageLayout(commandBuffer,
                dstTexture.getHandle(), dst_tex_layout,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}
}
