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
	 * GPU representation of a 2D image.
	 * This class does not own any CPU data, it offers an interface to upload / download texture data from and to a bitmap.
	 * When usage is set to 'Static' (default) or 'DynamicRead' data can be uploaded only once!
	 * Only when usage is set to 'DynamicWrite' can the texture be updated frequently from CPU to GPU.
	 */
	class NAPAPI Texture2D : public Resource
	{
		friend class RenderService;
		RTTI_ENABLE(Resource)
	public:
		Texture2D(Core& core);
		~Texture2D() override;

		/**
		 * Defines how the Texture2D is cleared on initialization.
		 */
		enum class EClearMode : uint8
		{
			DontClear	= 0,			///< Texture is created on GPU but not filled, GPU layout is undefined.
			Clear		= 1				///< Texture is created and cleared on the GPU.
		};

		/**
		* Creates the texture on the GPU using the provided settings.
		* The texture is initialized to 'clearColor' if 'clearMode' is set to 'FillWithZero'.
		* The Vulkan image usage flags are derived from texture usage.
		* @param descriptor texture description.
		* @param generateMipMaps if mip maps are generated when data is uploaded.
		* @param clearMode if the texture is immediately initialized to black after creation.
		* @param clearColor the color to clear the texture with.
		* @param requiredFlags image usage flags that are required, 0 = no additional usage flags.
		* @param errorState contains the error if the texture can't be initialized.
		* @return if the texture initialized successfully.
		*/
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, EClearMode clearMode, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);
		
		/**
		 * Creates the texture on the GPU using the provided settings.
		 * The texture is initialized to black if 'clearMode' is set to 'FillWithZero'.
		 * Otherwise the layout of the texture on the GPU will be undefined until upload.
		 * The Vulkan image usage flags are derived from texture usage. 
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip maps are generated when data is uploaded.
		 * @param clearMode if the texture is immediately initialized to black after creation.
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags.
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, EClearMode clearMode, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);
		
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
		 * @return Vulkan image view
		 */
		VkImageView getImageView() const					{ return mImageData.mTextureView; }

		/**
		 * @return number of mip-map levels
		 */
		int getMipmapCount()								{ return static_cast<int>(mMipLevels); }

		/**
		 * @return render service
		 */
		RenderService& getRenderService()					{ return *mRenderService; }

		/**
		 * Starts a transfer of texture data from GPU to CPU. 
		 * This is a non blocking call. When the transfer completes, the bitmap will be filled with the texture data.
		 * @param bitmap the bitmap to download texture data into.
		 */
		void asyncGetData(Bitmap& bitmap);

		ETextureUsage mUsage = ETextureUsage::Static;		///< Property: 'Usage' If this texture is updated frequently or considered static.

	private:
		// Test
		void clear(VkCommandBuffer commandBuffer);

		/**
		 * Called by the render service when data can be uploaded.
		 */
		void upload(VkCommandBuffer commandBuffer);

		/**
		 * Called by the render service when download is ready
		 */
		void notifyDownloadReady(int frameIndex);		

		/**
		 * Downloads texture data
		 */
		void download(VkCommandBuffer commandBuffer);
        
        // Hide resource init explicitly
        using Resource::init;

	protected:
		RenderService*						mRenderService = nullptr;

	private:
		using TextureReadCallback = std::function<void(void* data, size_t sizeInBytes)>;

		ImageData							mImageData;							///< 2D Texture vulkan image buffers
		std::vector<BufferData>				mStagingBuffers;					///< All vulkan staging buffers, 1 when static or using dynamic read, no. of frames in flight when dynamic write.
		int									mCurrentStagingBufferIndex = -1;	///< Currently used staging buffer
		size_t								mImageSizeInBytes = -1;				///< Size in bytes of texture
		SurfaceDescriptor					mDescriptor;						///< Texture description
		VkFormat							mFormat = VK_FORMAT_UNDEFINED;		///< Vulkan texture format
		std::vector<TextureReadCallback>	mReadCallbacks;						///< Number of callbacks based on number of frames in flight
		uint32								mMipLevels = 1;						///< Total number of generated mip-maps
		VkClearColorValue					mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };	///< Property: 'ClearColor' color selection used for clearing the texture
	};
}