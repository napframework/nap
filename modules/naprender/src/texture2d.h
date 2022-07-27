/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"
#include "renderutils.h"

// External Includes
#include <nap/resource.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>
#include <nap/numeric.h>
#include <rtti/factory.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class Bitmap;
	class RenderService;
	class Core;

	/**
	 * Flag that determines how the texture is used at runtime.
	 */
	enum class ETextureUsage
	{
		Static,				///< Texture does not change, uploaded to once
		DynamicRead,		///< Texture is frequently read from GPU to CPU
		DynamicWrite		///< Texture is frequently updated from CPU to GPU
	};


	/**
	 * GPU representation of a 2D image. This class does not own any CPU data.
	 * It offers the user an interface to upload & download texture data, from and to a bitmap.
	 * When usage is set to 'Static' (default) or 'DynamicRead' data can be uploaded only once.
	 * When usage is set to 'DynamicWrite' the texture be updated frequently from CPU to GPU.
	 */
	class NAPAPI Texture2D : public Resource
	{
		friend class RenderService;
		RTTI_ENABLE(Resource)
	public:
		Texture2D(Core& core);
		~Texture2D() override;

		/**
		 * Creates the texture on the GPU using the provided settings. The texture is cleared to 'ClearColor'.
		 * The Vulkan image usage flags are derived from texture usage.
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip maps are generated when data is uploaded.
		 * @param clearColor the color to clear the texture with.
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags.
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);
		
		/**
		 * Creates the texture on the GPU using the provided settings. The texture is cleared to 'ClearColor'.
		 * Otherwise the layout of the texture on the GPU will be undefined until upload.
		 * The Vulkan image usage flags are derived from texture usage. 
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip maps are generated when data is uploaded.
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags.
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);
		
		/**
		 * Creates the texture on the GPU using the provided settings and immediately requests a content upload.
		 * The Vulkan image usage flags are derived from texture usage.
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip maps are generated when data is uploaded.
		 * @param initialData the data to upload, must be of size SurfaceDescriptor::getSizeInBytes().
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, void* initialData, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);

		/**
		 * @return size of the texture in texels.
		 */
		const glm::vec2 getSize() const; 

		/**
		 *	@return width of the texture in texels
		 */
		int getWidth() const;

		/**
		 *	@return height of the texture in texels
		 */
		int getHeight() const;

		/**
		 * @return the texture description
		 */
		const SurfaceDescriptor& getDescriptor() const;

		/**
		 * Uploads CPU data to the texture on the GPU. 
		 * Note that you can only update the contents of a texture once if 'Usage' is 'DynamicRead' or 'Static'.
		 * @param data pointer to the CPU data.
		 * @param width width of the image in pixels
		 * @param height height of the image in pixels
		 * @param pitch size in bytes of a single row of pixel data.
		 * @param channels total number of channels: 3 for RGB, 4 for RGBA etc. 
		 */
		void update(const void* data, int width, int height, int pitch, ESurfaceChannels channels);

		/**
		 * Uploads CPU data to the texture on the GPU.
		 * Note that you can only update the contents of a texture once if 'Usage' is 'DynamicRead' or 'Static'.
		 * @param data pointer to the CPU data.
		 * @param surfaceDescriptor texture description.
		 */
		void update(const void* data, const SurfaceDescriptor& surfaceDescriptor);

		/**
		 * @return Vulkan texture format
		 */
		VkFormat getFormat() const							{ return mFormat; }

		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		const ImageData& getHandle() const					{ return mImageData; }

		/**
		 * @return Vulkan image layout
		 */
		virtual VkImageLayout getImageLayout() const		{ return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }

		/**
		 * @return number of mip-map levels
		 */
		int getMipmapCount()								{ return static_cast<int>(mMipLevels); }

		/**
		 * @return render service
		 */
		RenderService& getRenderService()					{ return *mRenderService; }

		/**
		 * @return render service
		 */
		const RenderService& getRenderService() const		{ return *mRenderService; }

		/**
		 * Starts a transfer of texture data from GPU to CPU. 
		 * This is a non blocking call. When the transfer completes, the bitmap will be filled with the texture data.
		 * @param bitmap the bitmap to download texture data into.
		 */
		void asyncGetData(Bitmap& bitmap);

		/**
		 * Starts a transfer of texture data from GPU to CPU. Use this overload to pass your own copy function.
		 * This is a non blocking call. When the transfer completes, the bitmap will be filled with the texture data.
		 * @param copyFunction the copy function to call when the texture data is available for download.
		 */
		void asyncGetData(std::function<void(const void*, size_t)> copyFunction);

		/**
		 * @return Handle to Vulkan image view
		 */
		VkImageView getImageView() const					{ return mImageData.mView; }

		ETextureUsage mUsage = ETextureUsage::Static;		///< Property: 'Usage' If this texture is updated frequently or considered static.

	protected:
		RenderService* mRenderService = nullptr;

	private:
		/**
		 * Creates the texture on the GPU using the provided settings.
		 * The Vulkan image usage flags are derived from texture usage.
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip maps are generated when data is uploaded.
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool initInternal(const SurfaceDescriptor& descriptor, bool generateMipMaps, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);

		/**
		* Clears the texture to the specified clear colors
		*/
		void clear(VkCommandBuffer commandBuffer);

		/**
		 * Called by the render service when data can be uploaded.
		 */
		void upload(VkCommandBuffer commandBuffer);

		/**
		 * Downloads texture data
		 */
		void download(VkCommandBuffer commandBuffer);

		/**
		 * Called by the render service when download is ready
		 */
		void notifyDownloadReady(int frameIndex);

		/**
		 * Clears queued texture downloads
		 */
		void clearDownloads();

        // Hide default resource init. Use specialized initialization functions instead.
        using Resource::init;

		using TextureReadCallback = std::function<void(void* data, size_t sizeInBytes)>;

		ImageData							mImageData;									///< 2D Texture vulkan image buffers
		std::vector<BufferData>				mStagingBuffers;							///< All vulkan staging buffers, 1 when static or using dynamic read, no. of frames in flight when dynamic write.
		int									mCurrentStagingBufferIndex = -1;			///< Currently used staging buffer
		size_t								mImageSizeInBytes = -1;						///< Size in bytes of texture
		SurfaceDescriptor					mDescriptor;								///< Texture description
		VkFormat							mFormat = VK_FORMAT_UNDEFINED;				///< Vulkan texture format
		std::vector<TextureReadCallback>	mReadCallbacks;								///< Number of callbacks based on number of frames in flight
		std::vector<int>					mDownloadStagingBufferIndices;				///< Staging buffer indices associated with a frameindex
		uint32								mMipLevels = 1;								///< Total number of generated mip-maps
		VkClearColorValue					mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };	///< Color used for clearing the texture
	};
}
