/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "texture2d.h"
#include "bitmap.h"
#include "renderservice.h"
#include "copyimagedata.h"
#include "textureutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <glm/gtc/type_ptr.hpp>

RTTI_BEGIN_ENUM(nap::ETextureUsage)
	RTTI_ENUM_VALUE(nap::ETextureUsage::Static,			"Static"),
	RTTI_ENUM_VALUE(nap::ETextureUsage::DynamicRead,	"DynamicRead"),
	RTTI_ENUM_VALUE(nap::ETextureUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

// Texture2D class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Texture2D)
	RTTI_PROPERTY("Usage", 			&nap::Texture2D::mUsage,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkImageAspectFlags aspect, uint32_t width, uint32_t height)
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}


	static void copyImageToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkBuffer buffer, VkImageAspectFlags aspect, uint32_t width, uint32_t height)
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);
	}


	static int getNumStagingBuffers(int maxFramesInFlight, ETextureUsage textureUsage)
	{
		switch (textureUsage)
		{
			case ETextureUsage::DynamicWrite:
				return maxFramesInFlight + 1;
			case ETextureUsage::Static:
				return 1;
			case ETextureUsage::DynamicRead:
				return maxFramesInFlight;
			default:
				assert(false);
		}
		return 0;
	}


	static void createMipmaps(VkCommandBuffer buffer, VkImage image, VkFormat imageFormat, VkImageLayout targetLayout, VkImageAspectFlags aspect, uint32 texWidth, uint32 texHeight, uint32 mipLevels)
	{
		int32 mipWidth  = static_cast<int32>(texWidth);
		int32 mipHeight = static_cast<int32>(texHeight);

		VkImageMemoryBarrier barrier {};
		for (uint32_t i = 1; i < mipLevels; i++)
		{
			// Prepare LOD for blit operation
			transitionImageLayout(buffer, image, barrier,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_ACCESS_TRANSFER_WRITE_BIT,			VK_ACCESS_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_TRANSFER_BIT,
				i - 1,									1,
				aspect);

			// Create blit structure
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = aspect;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = aspect;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			// Blit
			vkCmdBlitImage(buffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			// Prepare LOD for shader read
			transitionImageLayout(buffer, image, barrier,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,	targetLayout,
				VK_ACCESS_TRANSFER_READ_BIT,			VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				i - 1,									1,
				aspect);

			if (mipWidth  > 1) mipWidth  /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// Prepare final LOD for shader read
		transitionImageLayout(buffer, image, barrier,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	targetLayout,
			VK_ACCESS_TRANSFER_WRITE_BIT,			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			mipLevels - 1,							1,
			aspect);
	}


	//////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////

	Texture2D::Texture2D(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}


	Texture2D::~Texture2D()
	{	
		// Remove all previously made requests and queue buffers for destruction.
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService->removeTextureRequests(*this);
		mRenderService->queueVulkanObjectDestructor([imageData = mImageData, stagingBuffers = mStagingBuffers](RenderService& renderService) mutable
		{
			destroyImageAndView(imageData, renderService.getDevice(), renderService.getVulkanAllocator());
			for (BufferData& buffer : stagingBuffers)
			{
				destroyBuffer(renderService.getVulkanAllocator(), buffer);
			}
		});
	}


	bool Texture2D::initInternal(const SurfaceDescriptor& descriptor, bool generateMipMaps, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		// Get the format, when unsupported bail.
		mFormat = getTextureFormat(descriptor);
		if (!errorState.check(mFormat != VK_FORMAT_UNDEFINED, "%s, Unsupported texture format", mID.c_str()))
			return false;

		// Ensure our GPU image can be used as a transfer destination during uploads
		VkFormatProperties format_properties;
		mRenderService->getFormatProperties(mFormat, format_properties);
		if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
		{
			errorState.fail("%s: image format does not support being used as a transfer destination", mID.c_str());
			return false;
		}

		// If mip mapping is enabled, ensure it is supported
		if (generateMipMaps)
		{
			if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			{
				errorState.fail("%s: image format does not support linear blitting, consider disabling mipmap generation", mID.c_str());
				return false;
			}
			mMipLevels = static_cast<uint32>(std::floor(std::log2(std::max(descriptor.getWidth(), descriptor.getHeight())))) + 1;
		}

		// Ensure there are enough read callbacks based on max number of frames in flight
		mImageSizeInBytes = descriptor.getSizeInBytes();
		if (mUsage == ETextureUsage::DynamicRead)
		{
			mReadCallbacks.resize(mRenderService->getMaxFramesInFlight());
			mDownloadStagingBufferIndices.resize(mRenderService->getMaxFramesInFlight());
		}

		// Here we create staging buffers. Client data is copied into staging buffers. The staging buffers are then used as a source to update
		// the GPU texture. The updating of the GPU textures is done on the command buffer. The updating of the staging buffers can be done
		// at any time. However, as the staging buffers serve as a source for updating the GPU buffers, they are part of the command buffer.
		// 
		// We can only safely update the staging buffer if we know it isn't used anymore. We generally make enough resources for each frame
		// that can be in flight. Once we've passed RenderService::beginRendering, we know that the resources for the current frame are 
		// not in use anymore. If we would use this strategy, we could only safely use a staging buffer during rendering. To be more 
		// specific, we could only use the staging buffer during rendering, but before the render pass was set (as this is a Vulkan
		// requirement for buffer transfers). This is very inconvenient for texture updating, as we'd ideally like to update texture contents
		// at any point in the frame. We also don't want to make an extra copy of the texture that would be used during rendering. To solve 
		// this problem, we use one additional staging buffer. This guarantees that there's always a single staging buffer free at any point 
		// in the frame. So the amount of staging buffers is:  'maxFramesInFlight' + 1. Updating the staging buffer multiple times within a 
		// frame will just overwrite the same staging buffer.
		//
		// A final note: this system is built to be able to handle changing the texture every frame. But if the texture is changed less frequently,
		// or never, that works as well. When update is called, the RenderService is notified of the change, and during rendering, the upload is
		// called, which moves the index one place ahead.
		VmaAllocator vulkan_allocator = mRenderService->getVulkanAllocator();

		// When read frequently, the buffer is a destination, otherwise used as a source for texture upload
		VkBufferUsageFlags buffer_usage = mUsage == ETextureUsage::DynamicRead ?
			VK_BUFFER_USAGE_TRANSFER_DST_BIT :
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// When read frequently, the buffer receives from the GPU, otherwise the buffer receives from CPU
		VmaMemoryUsage memory_usage = mUsage == ETextureUsage::DynamicRead ?
			VMA_MEMORY_USAGE_GPU_TO_CPU :
			VMA_MEMORY_USAGE_CPU_TO_GPU;

		mStagingBuffers.resize(getNumStagingBuffers(mRenderService->getMaxFramesInFlight(), mUsage));
		for (int index = 0; index < mStagingBuffers.size(); ++index)
		{
			BufferData& staging_buffer = mStagingBuffers[index];

			// Create staging buffer
			if (!createBuffer(vulkan_allocator, mImageSizeInBytes, buffer_usage, memory_usage, 0, staging_buffer, errorState))
			{
				errorState.fail("%s: Unable to create staging buffer for texture", mID.c_str());
				return false;
			}
		}

		// Set image usage flags: can be written to, read and sampled
		VkImageUsageFlags usage = requiredFlags;
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Create GPU image
		if (!create2DImage(vulkan_allocator, descriptor.mWidth, descriptor.mHeight, mFormat, mMipLevels,
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, VMA_MEMORY_USAGE_GPU_ONLY, mImageData.mImage, mImageData.mAllocation, mImageData.mAllocationInfo, errorState))
			return false;

		// Check whether the texture is flagged as depth
		bool is_depth = descriptor.getChannels() == ESurfaceChannels::D;

		// Create GPU image view
		VkImageAspectFlags aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (!create2DImageView(mRenderService->getDevice(), mImageData.getImage(), mFormat, mMipLevels, aspect, mImageData.mView, errorState))
			return false;

		// Initialize buffer indexing
		mCurrentStagingBufferIndex = 0;
		mDescriptor = descriptor;

		return true;
	}



	bool Texture2D::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		if (!initInternal(descriptor, generateMipMaps, requiredFlags, errorState))
			return false;

		// Clear the texture and perform a layout transition to shader read
		mRenderService->requestTextureClear(*this);
		return true;
	}


	bool Texture2D::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		if (!initInternal(descriptor, generateMipMaps, requiredFlags, errorState))
			return false;

		// Set clear color
		std::memcpy(&mClearColor, glm::value_ptr(clearColor), sizeof(VkClearColorValue));

		// Clear the texture and perform a layout transition to shader read
		mRenderService->requestTextureClear(*this);
		return true;
	}


	bool Texture2D::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, void* initialData, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		if (!initInternal(descriptor, generateMipMaps, requiredFlags, errorState))
			return false;

		// Upload initial data and perform a layout transition to shader read
		update(initialData, descriptor);
		return true;
	}


	const glm::vec2 Texture2D::getSize() const
	{
		return glm::vec2(getWidth(), getHeight());
	}


	int Texture2D::getWidth() const
	{
		return mDescriptor.mWidth;
	}


	int Texture2D::getHeight() const
	{
		return mDescriptor.mHeight;
	}


	const nap::SurfaceDescriptor& Texture2D::getDescriptor() const
	{
		return mDescriptor;
	}


	void Texture2D::clear(VkCommandBuffer commandBuffer)
	{
		// Do not clear if this is an attachment
		// Attachments use vkCmdClearAttachments for clear operations
		// But should really be cleared in a renderpass with VK_ATTACHMENT_LOAD_OP_CLEAR
		if (getImageLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || getImageLayout() == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || getImageLayout() > VK_IMAGE_LAYOUT_PREINITIALIZED)
			return;

		// Texture clear commands are the first subset of commands to be pushed to the upload command buffer at the beginning of a frame
		// Therefore, the initial layout transition waits for nothing
		VkAccessFlags srcMask = 0;
		VkAccessFlags dstMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		if (mImageData.mCurrentLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			srcMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		// Get image ready for clear, applied to all mipmap layers
		VkImageAspectFlags aspect = mDescriptor.getChannels() == ESurfaceChannels::D ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		transitionImageLayout(commandBuffer, mImageData.mImage,
			mImageData.mCurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			srcMask, dstMask,
			srcStage, dstStage,
			0, mMipLevels, aspect);

		VkImageSubresourceRange image_subresource_range = {};
		image_subresource_range.aspectMask = aspect;
		image_subresource_range.baseMipLevel = 0;
		image_subresource_range.levelCount = mMipLevels;
		image_subresource_range.baseArrayLayer = 0;
		image_subresource_range.layerCount = 1;

		// Clear color or depth/stencil
		if (mDescriptor.getChannels() != ESurfaceChannels::D)
		{
			vkCmdClearColorImage(commandBuffer, mImageData.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &mClearColor, 1, &image_subresource_range);
		}
		else
		{
			VkClearDepthStencilValue clear_depthstencil = { 1.0f, 0 };
			vkCmdClearDepthStencilImage(commandBuffer, mImageData.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depthstencil, 1, &image_subresource_range);
		}

		// Transition image layout
		transitionImageLayout(commandBuffer, mImageData.mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, getImageLayout(),
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 1, aspect);

		// We store the last image layout, which is used as input for a subsequent upload
		mImageData.mCurrentLayout = getImageLayout();
	}


	void Texture2D::upload(VkCommandBuffer commandBuffer)
	{
		assert(mCurrentStagingBufferIndex != -1);
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];
		assert(buffer.mAllocation != VK_NULL_HANDLE);
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		// Texture uploads are recorded after clear commands, and we do not want operations on the same texture to interfere with each other
		// Therefore, the initial layout transition requires synchronization with a potential prior clear command
		VkAccessFlags srcMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		VkAccessFlags dstMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		if (mImageData.mCurrentLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			srcMask  |= VK_ACCESS_SHADER_READ_BIT;
			srcStage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		// Get image ready for copy, applied to all mipmap layers
		VkImageAspectFlags aspect = mDescriptor.getChannels() == ESurfaceChannels::D ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		transitionImageLayout(commandBuffer, mImageData.mImage, 
			mImageData.mCurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			srcMask,	dstMask,
			srcStage,	dstStage,
			0,			mMipLevels,
			aspect);
		
		// Copy staging buffer to image
		copyBufferToImage(commandBuffer, buffer.mBuffer, mImageData.mImage, aspect, mDescriptor.mWidth, mDescriptor.mHeight);
		
		// Generate mip maps, if we do that we don't have to transition the image layout anymore, this is handled by createMipmaps.
		if (mMipLevels > 1)
		{
			createMipmaps(commandBuffer, mImageData.mImage, mFormat, getImageLayout(), aspect, mDescriptor.mWidth, mDescriptor.mHeight, mMipLevels);
		}
		else
		{
			transitionImageLayout(commandBuffer, mImageData.mImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_ACCESS_TRANSFER_WRITE_BIT,			VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,										1,
				aspect);
		}

		// We store the last image layout, which is used as input for a subsequent upload
		mImageData.mCurrentLayout = getImageLayout();

		// Destroy staging buffer when usage is static
		// This queues the vulkan staging resource for destruction, executed by the render service at the appropriate time.
		// Explicitly release the handle, so it's not deleted twice.
		if (mUsage == ETextureUsage::Static)
		{
			assert(mStagingBuffers.size() == 1);
			mRenderService->queueVulkanObjectDestructor([del_buffer = buffer](RenderService& renderService) mutable
			{
				destroyBuffer(renderService.getVulkanAllocator(), del_buffer);
			});
			buffer.release();
		}
	}


	void Texture2D::download(VkCommandBuffer commandBuffer)
	{
		assert(mCurrentStagingBufferIndex != -1);
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Store the staging buffer index associated with the download in the current frame for lookup later
		mDownloadStagingBufferIndices[mRenderService->getCurrentFrameIndex()] = mCurrentStagingBufferIndex;
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		// Transition for copy
		VkImageAspectFlags aspect = mDescriptor.getChannels() == ESurfaceChannels::D ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		transitionImageLayout(commandBuffer, mImageData.mImage, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_SHADER_WRITE_BIT,					VK_ACCESS_TRANSFER_READ_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,		VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,											1,
			aspect);
		
		// Copy to buffer
		copyImageToBuffer(commandBuffer, mImageData.mImage, buffer.mBuffer, aspect, mDescriptor.mWidth, mDescriptor.mHeight);
		
		// Transition back to shader usage
		transitionImageLayout(commandBuffer, mImageData.mImage, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_READ_BIT,				VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,											1,
			aspect);
	}


	void Texture2D::update(const void* data, const SurfaceDescriptor& surfaceDescriptor)
	{
		update(data, surfaceDescriptor.getWidth(), surfaceDescriptor.getHeight(), surfaceDescriptor.getPitch(), surfaceDescriptor.getChannels());
	}


	void Texture2D::update(const void* data, int width, int height, int pitch, ESurfaceChannels channels)
	{
		// We can only upload when the texture usage is dynamic, OR this is the first upload for a static texture
		assert(mUsage == ETextureUsage::DynamicWrite || mImageData.mCurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED);
		assert(mDescriptor.mWidth == width && mDescriptor.mHeight == height);

		// We use a staging buffer that is guaranteed to be free
		assert(mCurrentStagingBufferIndex != -1);
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Update the staging buffer using the Bitmap contents
		VmaAllocator vulkan_allocator = mRenderService->getVulkanAllocator();

		// Map memory and copy contents, note for this to work on OSX the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is required!
		void* mapped_memory = nullptr;
		VkResult result = vmaMapMemory(vulkan_allocator, buffer.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);
		copyImageData((const uint8_t*)data, pitch, channels, (uint8_t*)mapped_memory, mDescriptor.getPitch(), mDescriptor.mChannels, mDescriptor.mWidth, mDescriptor.mHeight);
		vmaUnmapMemory(vulkan_allocator, buffer.mAllocation);

		// Notify the RenderService that it should upload the texture contents during rendering
		mRenderService->requestTextureUpload(*this);
	}


	void Texture2D::asyncGetData(Bitmap& bitmap)
	{
 		assert(!mReadCallbacks[mRenderService->getCurrentFrameIndex()]);
 		mReadCallbacks[mRenderService->getCurrentFrameIndex()] = [this, &bitmap](const void* data, size_t sizeInBytes)
		{
			// Check if initialization is necessary
			if (bitmap.empty() || bitmap.mSurfaceDescriptor != mDescriptor) {
				bitmap.initFromDescriptor(mDescriptor);
			}
			memcpy(bitmap.getData(), data, sizeInBytes);
			bitmap.mBitmapUpdated();
 		};
		mRenderService->requestTextureDownload(*this);
	}


	void Texture2D::asyncGetData(std::function<void(const void*, size_t)> copyFunction)
	{
		assert(!mReadCallbacks[mRenderService->getCurrentFrameIndex()]);
		mReadCallbacks[mRenderService->getCurrentFrameIndex()] = copyFunction;
		mRenderService->requestTextureDownload(*this);
	}


	void Texture2D::clearDownloads()
	{
		for (auto& callback : mReadCallbacks)
			callback = TextureReadCallback();
	}


	void Texture2D::notifyDownloadReady(int frameIndex)
	{
		// Update the staging buffer using the Bitmap contents
		VmaAllocator vulkan_allocator = mRenderService->getVulkanAllocator();

		// Copy data, not for this to work the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is required on OSX!
		int downloaded_staging_buffer_index = mDownloadStagingBufferIndices[frameIndex];
		BufferData& buffer = mStagingBuffers[downloaded_staging_buffer_index];

		void* mapped_memory = nullptr;
		VkResult result = vmaMapMemory(vulkan_allocator, buffer.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);

		mReadCallbacks[frameIndex](mapped_memory, mImageSizeInBytes);
		vmaUnmapMemory(vulkan_allocator, buffer.mAllocation);
		mReadCallbacks[frameIndex] = TextureReadCallback();
	}
}
