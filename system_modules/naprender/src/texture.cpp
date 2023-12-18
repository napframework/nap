/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "texture.h"
#include "bitmap.h"
#include "renderservice.h"
#include "copyimagedata.h"
#include "textureutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <glm/gtc/type_ptr.hpp>

RTTI_BEGIN_ENUM(nap::Texture::EUsage)
	RTTI_ENUM_VALUE(nap::Texture::EUsage::Static,		"Static"),
	RTTI_ENUM_VALUE(nap::Texture::EUsage::DynamicRead,	"DynamicRead"),
	RTTI_ENUM_VALUE(nap::Texture::EUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

// Define Texture base
RTTI_DEFINE_BASE(nap::Texture)

// Texture2D class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Texture2D)
	RTTI_PROPERTY("Usage", 			&nap::Texture2D::mUsage,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// TextureCube class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TextureCube)
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


	static int getNumStagingBuffers(int maxFramesInFlight, Texture::EUsage textureUsage)
	{
		switch (textureUsage)
		{
		case Texture::EUsage::DynamicWrite:
				return maxFramesInFlight + 1;
			case Texture::EUsage::Static:
				return 1;
			case Texture::EUsage::DynamicRead:
				return maxFramesInFlight;
			default:
				assert(false);
		}
		return 0;
	}


	//////////////////////////////////////////////////////////////////////////
	// Texture
	//////////////////////////////////////////////////////////////////////////

	Texture::Texture(Core& core) :
		mRenderService(*core.getService<RenderService>())
	{ }


	void Texture::clear(VkCommandBuffer commandBuffer)
	{
		// Do not clear if this is an attachment
		// Attachments use vkCmdClearAttachments for clear operations
		// But should really be cleared in a renderpass with VK_ATTACHMENT_LOAD_OP_CLEAR
		if (getTargetLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || getTargetLayout() == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || getTargetLayout() > VK_IMAGE_LAYOUT_PREINITIALIZED)
			return;

		// Texture clear commands are the first subset of commands to be pushed to the upload command buffer at the beginning of a frame
		// Therefore, the initial layout transition waits for nothing
		VkAccessFlags srcMask = 0;
		VkAccessFlags dstMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		if (getHandle().mCurrentLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			srcMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		// Get image ready for clear, applied to all mipmap layers
		VkImageAspectFlags aspect = mDescriptor.getChannels() == ESurfaceChannels::D ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		utility::transitionImageLayout(commandBuffer, getHandle().mImage,
			getHandle().mCurrentLayout,		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			srcMask,						dstMask,
			srcStage,						dstStage,
			0,								getMipLevels(),
			aspect);

		VkImageSubresourceRange image_subresource_range = { aspect, 0, getMipLevels(), 0, getLayerCount() };

		// Clear color or depth/stencil
		if (mDescriptor.getChannels() != ESurfaceChannels::D)
		{
			vkCmdClearColorImage(commandBuffer, getHandle().mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &mClearColor, 1, &image_subresource_range);
		}
		else
		{
			VkClearDepthStencilValue clear_depthstencil = { 1.0f, 0 };
			vkCmdClearDepthStencilImage(commandBuffer, getHandle().mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depthstencil, 1, &image_subresource_range);
		}

		// Transition image layout
		utility::transitionImageLayout(commandBuffer, getHandle().mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	getTargetLayout(),
			VK_ACCESS_TRANSFER_WRITE_BIT,			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,										getMipLevels(),
			aspect);

		// We store the last image layout, which is used as input for a subsequent upload
		getHandle().mCurrentLayout = getTargetLayout();
	}


	void Texture::requestClear()
	{
		mRenderService.requestTextureClear(*this);
	}


	//////////////////////////////////////////////////////////////////////////
	// Texture2D
	//////////////////////////////////////////////////////////////////////////

	Texture2D::Texture2D(Core& core) :
		Texture(core)
	{ }


	Texture2D::~Texture2D()
	{	
		// Remove all previously made requests and queue buffers for destruction.
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService.removeTextureRequests(*this);
		mRenderService.queueVulkanObjectDestructor([imageData = mImageData, stagingBuffers = mStagingBuffers](RenderService& renderService) mutable
		{
			utility::destroyImageAndView(imageData, renderService.getDevice(), renderService.getVulkanAllocator());
			for (BufferData& buffer : stagingBuffers)
			{
				utility::destroyBuffer(renderService.getVulkanAllocator(), buffer);
			}
		});
	}


	bool Texture2D::initInternal(const SurfaceDescriptor& descriptor, bool generateMipMaps, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		// Get the format, when unsupported bail.
		mFormat = utility::getTextureFormat(descriptor);
		if (!errorState.check(mFormat != VK_FORMAT_UNDEFINED, "%s, Unsupported texture format", mID.c_str()))
			return false;

		// Ensure our GPU image can be used as a transfer destination during uploads
		VkFormatProperties format_properties;
		mRenderService.getFormatProperties(mFormat, format_properties);
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
		if (mUsage == EUsage::DynamicRead)
		{
			mReadCallbacks.resize(mRenderService.getMaxFramesInFlight());
			mDownloadStagingBufferIndices.resize(mRenderService.getMaxFramesInFlight());
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
		VmaAllocator vulkan_allocator = mRenderService.getVulkanAllocator();

		// When read frequently, the buffer is a destination, otherwise used as a source for texture upload
		VkBufferUsageFlags buffer_usage = mUsage == EUsage::DynamicRead ?
			VK_BUFFER_USAGE_TRANSFER_DST_BIT :
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// When read frequently, the buffer receives from the GPU, otherwise the buffer receives from CPU
		VmaMemoryUsage memory_usage = mUsage == EUsage::DynamicRead ?
			VMA_MEMORY_USAGE_GPU_TO_CPU :
			VMA_MEMORY_USAGE_CPU_TO_GPU;

		mStagingBuffers.resize(getNumStagingBuffers(mRenderService.getMaxFramesInFlight(), mUsage));
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
		VkImageUsageFlags usage = requiredFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Create GPU image
		if (!create2DImage(vulkan_allocator, descriptor.mWidth, descriptor.mHeight, mFormat, mMipLevels,
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, VMA_MEMORY_USAGE_GPU_ONLY, mImageData.mImage, mImageData.mAllocation, mImageData.mAllocationInfo, errorState))
			return false;

		// Check whether the texture is flagged as depth
		bool is_depth = descriptor.getChannels() == ESurfaceChannels::D;

		// Create GPU image view
		VkImageAspectFlags aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (!create2DImageView(mRenderService.getDevice(), mImageData.getImage(), mFormat, mMipLevels, aspect, mImageData.getView(), errorState))
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
		requestClear();
		return true;
	}


	bool Texture2D::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		if (!initInternal(descriptor, generateMipMaps, requiredFlags, errorState))
			return false;

		// Set clear color
		std::memcpy(&mClearColor, glm::value_ptr(clearColor), sizeof(VkClearColorValue));

		// Clear the texture and perform a layout transition to shader read
		requestClear();
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
		utility::transitionImageLayout(commandBuffer, mImageData.mImage,
			mImageData.mCurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			srcMask,			dstMask,
			srcStage,			dstStage,
			0,					mMipLevels,
			aspect);
		
		// Copy staging buffer to image
		copyBufferToImage(commandBuffer, buffer.mBuffer, mImageData.mImage, aspect, mDescriptor.mWidth, mDescriptor.mHeight);
		
		// Generate mip maps, if we do that we don't have to transition the image layout anymore, this is handled by createMipmaps.
		if (mMipLevels > 1)
		{
			utility::createMipmaps(commandBuffer, mImageData.mImage, mFormat, getTargetLayout(), aspect, mDescriptor.mWidth, mDescriptor.mHeight, mMipLevels);
		}
		else
		{
			utility::transitionImageLayout(commandBuffer, mImageData.mImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	getTargetLayout(),
				VK_ACCESS_TRANSFER_WRITE_BIT,			VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,										1,
				aspect);
		}

		// We store the last image layout, which is used as input for a subsequent upload
		mImageData.mCurrentLayout = getTargetLayout();

		// Destroy staging buffer when usage is static
		// This queues the vulkan staging resource for destruction, executed by the render service at the appropriate time.
		// Explicitly release the handle, so it's not deleted twice.
		if (mUsage == EUsage::Static)
		{
			assert(mStagingBuffers.size() == 1);
			mRenderService.queueVulkanObjectDestructor([del_buffer = buffer](RenderService& renderService) mutable
			{
				utility::destroyBuffer(renderService.getVulkanAllocator(), del_buffer);
			});
			buffer.release();
		}
	}


	void Texture2D::download(VkCommandBuffer commandBuffer)
	{
		assert(mCurrentStagingBufferIndex != -1);
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Store the staging buffer index associated with the download in the current frame for lookup later
		mDownloadStagingBufferIndices[mRenderService.getCurrentFrameIndex()] = mCurrentStagingBufferIndex;
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		// Transition for copy
		VkImageAspectFlags aspect = mDescriptor.getChannels() == ESurfaceChannels::D ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		utility::transitionImageLayout(commandBuffer, mImageData.mImage,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_SHADER_WRITE_BIT,					VK_ACCESS_TRANSFER_READ_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,		VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,											1,
			aspect);
		
		// Copy to buffer
		copyImageToBuffer(commandBuffer, mImageData.mImage, buffer.mBuffer, aspect, mDescriptor.mWidth, mDescriptor.mHeight);
		
		// Transition back to shader usage
		utility::transitionImageLayout(commandBuffer, mImageData.mImage,
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
		assert(mUsage == EUsage::DynamicWrite || mImageData.mCurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED);
		assert(mDescriptor.mWidth == width && mDescriptor.mHeight == height);

		// We use a staging buffer that is guaranteed to be free
		assert(mCurrentStagingBufferIndex != -1);
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Update the staging buffer using the Bitmap contents
		VmaAllocator vulkan_allocator = mRenderService.getVulkanAllocator();

		// Map memory and copy contents, note for this to work on OSX the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is required!
		void* mapped_memory = nullptr;
		VkResult result = vmaMapMemory(vulkan_allocator, buffer.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);
		copyImageData((const uint8_t*)data, pitch, channels, (uint8_t*)mapped_memory, mDescriptor.getPitch(), mDescriptor.mChannels, mDescriptor.mWidth, mDescriptor.mHeight);
		vmaUnmapMemory(vulkan_allocator, buffer.mAllocation);

		// Notify the RenderService that it should upload the texture contents during rendering
		mRenderService.requestTextureUpload(*this);
	}


	void Texture2D::asyncGetData(Bitmap& bitmap)
	{
 		assert(!mReadCallbacks[mRenderService.getCurrentFrameIndex()]);
 		mReadCallbacks[mRenderService.getCurrentFrameIndex()] = [this, &bitmap](const void* data, size_t sizeInBytes)
		{
			// Check if initialization is necessary
			if (bitmap.empty() || bitmap.mSurfaceDescriptor != mDescriptor) {
				bitmap.initFromDescriptor(mDescriptor);
			}
			memcpy(bitmap.getData(), data, sizeInBytes);
			bitmap.mBitmapUpdated();
 		};
		mRenderService.requestTextureDownload(*this);
	}


	void Texture2D::asyncGetData(std::function<void(const void*, size_t)> copyFunction)
	{
		assert(!mReadCallbacks[mRenderService.getCurrentFrameIndex()]);
		mReadCallbacks[mRenderService.getCurrentFrameIndex()] = copyFunction;
		mRenderService.requestTextureDownload(*this);
	}


	void Texture2D::clearDownloads()
	{
		for (auto& callback : mReadCallbacks)
			callback = TextureReadCallback();
	}


	void Texture2D::notifyDownloadReady(int frameIndex)
	{
		// Update the staging buffer using the Bitmap contents
		VmaAllocator vulkan_allocator = mRenderService.getVulkanAllocator();

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


	//////////////////////////////////////////////////////////////////////////
	// TextureCube
	//////////////////////////////////////////////////////////////////////////

	TextureCube::TextureCube(Core& core) :
		Texture(core)
	{ }


	TextureCube::~TextureCube()
	{
		// If the service is not running, all objects are destroyed immediately.
		// Otherwise they are destroyed when they are guaranteed not to be in use by the GPU.
		mRenderService.queueVulkanObjectDestructor([imageData = mImageData](RenderService& renderService) mutable
		{
			utility::destroyImageAndView(imageData, renderService.getDevice(), renderService.getVulkanAllocator());
		});
	}


	bool TextureCube::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
	{
		// Get the format, when unsupported bail.
		mFormat = utility::getTextureFormat(descriptor);
		if (!errorState.check(mFormat != VK_FORMAT_UNDEFINED, "%s, Unsupported texture format", mID.c_str()))
			return false;

		// Ensure our GPU image can be used as a transfer destination during uploads
		VkFormatProperties format_properties;
		mRenderService.getFormatProperties(mFormat, format_properties);
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

		// Set image usage flags: can be written to, read and sampled
		mImageUsageFlags = requiredFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Create GPU image
		VmaAllocator vulkan_allocator = mRenderService.getVulkanAllocator();
		if (!createLayered2DImage(vulkan_allocator, descriptor.mWidth, descriptor.mHeight, mFormat, mMipLevels, getLayerCount(),
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, mImageUsageFlags, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, mImageData.mImage, mImageData.mAllocation, mImageData.mAllocationInfo, errorState))
			return false;

		// Check whether the texture is flagged as depth
		bool is_depth = descriptor.getChannels() == ESurfaceChannels::D;

		// Create GPU image view
		VkImageAspectFlags aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (!createCubeImageView(mRenderService.getDevice(), mImageData.getImage(), mFormat, mMipLevels, aspect, getLayerCount(), mImageData.mView, errorState))
			return false;

		for (uint i = 0; i < mImageData.getSubViewCount(); i++)
		{
			// Set mipLevels to 1 as we typically only render to the first mip level of each layer individually
			// After the render operation we can update the mip maps using a blit operation if desired
			if (!createLayered2DImageView(mRenderService.getDevice(), mImageData.getImage(), mFormat, 1, aspect, i, 1, mImageData.getSubView(i), errorState))
				return false;
		}

		mDescriptor = descriptor;

		// Set clear color
		std::memcpy(&mClearColor, glm::value_ptr(clearColor), sizeof(VkClearColorValue));

		// Clear the texture and perform a layout transition to shader read
		requestClear();
		return true;
	}
}
