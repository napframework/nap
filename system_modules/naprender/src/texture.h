/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"
#include "renderutils.h"

// External Includes
#include <nap/resource.h>
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
	 * Texture base class
	 */
	class NAPAPI Texture : public Resource
	{
		friend class RenderService;
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Flag that determines how the texture is used at runtime.
		 */
		enum class EUsage
		{
			Static,				///< Texture does not change, uploaded to once
			DynamicRead,		///< Texture is frequently read from GPU to CPU
			DynamicWrite		///< Texture is frequently updated from CPU to GPU
		};

		// Constructor
		Texture(Core& core);

		// Destructor
		virtual ~Texture() { };

		/**
		 * @return the number of texture layers.
		 */
		virtual uint getLayerCount() const = 0;

		/**
		 * @return the number of texture mip-map levels.
		 */
		virtual uint getMipLevels() const = 0;

		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual const ImageData& getHandle() const = 0;

		/**
		 * @return Vulkan image layout that this texture is intended for.
		 */
		virtual VkImageLayout getTargetLayout() const			{ return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; };

		/**
		 * @return Vulkan texture format
		 */
		VkFormat getFormat() const								{ return mFormat; }

		/**
		 * @return the texture description
		 */
		const SurfaceDescriptor& getDescriptor() const			{ return mDescriptor; }

		/**
		 * @return render service
		 */
		RenderService& getRenderService()						{ return mRenderService; }

		/**
		 * @return render service
		 */
		const RenderService& getRenderService() const			{ return mRenderService; }

		/**
		 * Notify listeners when texture is destroyed
		 */
		virtual void onDestroy() override						{ textureDestroyed(); }

		nap::Signal<> textureDestroyed;							///< Signal that is triggered before texture is destroyed

	protected:
		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual ImageData& getHandle() = 0;

		// Hide default resource init. Use specialized initialization functions instead.
		using Resource::init;

		/**
		 * Clears the texture to the specified clear colors.
		 */
		virtual void clear(VkCommandBuffer commandBuffer);

		/**
		 * Queues a clear command in the render service.
		 */
		void requestClear();

		RenderService&						mRenderService;								///< Reference to the render service
		SurfaceDescriptor					mDescriptor;								///< Texture description
		VkFormat							mFormat = VK_FORMAT_UNDEFINED;				///< Vulkan texture format
		VkClearColorValue					mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };	///< Color used for clearing the texture
	};


	/**
	 * GPU representation of a 2D image. This class does not own any CPU data.
	 * It offers the user an interface to upload & download texture data, from and to a bitmap.
	 * When usage is set to 'Static' (default) or 'DynamicRead' data can be uploaded only once.
	 * When usage is set to 'DynamicWrite' the texture be updated frequently from CPU to GPU.
	 */
	class NAPAPI Texture2D : public Texture
	{
		friend class RenderService;
		RTTI_ENABLE(Texture)
	public:
		Texture2D(Core& core);
		virtual ~Texture2D() override;

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
		const glm::vec2 getSize() const							{ return { getWidth(), getHeight() }; }

		/**
		 *	@return width of the texture in texels
		 */
		int getWidth() const									{ return mDescriptor.mWidth; }

		/**
		 *	@return height of the texture in texels
		 */
		int getHeight() const									{ return mDescriptor.mHeight; }

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
		 * @return the number of texture layers
		 */
		virtual uint getLayerCount() const override				{ return 1; }

		/**
		 * @return the number of texture mip-map levels
		 */
		virtual uint getMipLevels() const override				{ return mMipLevels; }

		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual const ImageData& getHandle() const override		{ return mImageData; }

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

		EUsage mUsage = EUsage::Static;							///< Property: 'Usage' If this texture is updated frequently or considered static.

	protected:
		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual ImageData& getHandle() override					{ return mImageData; }

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
		 * Called by the render service when data can be uploaded
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

		using TextureReadCallback = std::function<void(void* data, size_t sizeInBytes)>;

		ImageData							mImageData;									///< 2D Texture vulkan image buffers
		std::vector<BufferData>				mStagingBuffers;							///< All vulkan staging buffers, 1 when static or using dynamic read, no. of frames in flight when dynamic write.
		int									mCurrentStagingBufferIndex = -1;			///< Currently used staging buffer
		size_t								mImageSizeInBytes = -1;						///< Size in bytes of texture
		std::vector<TextureReadCallback>	mReadCallbacks;								///< Number of callbacks based on number of frames in flight
		std::vector<int>					mDownloadStagingBufferIndices;				///< Staging buffer indices associated with a frameindex
		uint32								mMipLevels = 1;								///< Total number of generated mip-maps
	};


	/**
	 * Cube texture base class.
	 * A cube texture is a six-layer image, where each layer represents one side
	 *
	 * Cube image layers are addressed and oriented as follows:
	 * - Layer 0: right (+X)
	 * - Layer 1: left (-X)
	 * - Layer 2: up (+Y)
	 * - Layer 3: down (-Y)
	 * - Layer 4: back (+Z)
	 * - Layer 5: forward (-Z)
	 */
	class NAPAPI TextureCube : public Texture
	{
		friend class RenderService;
		RTTI_ENABLE(Texture)
	public:
		// The image layer count is equal to the number of sides of a cube
		static constexpr const uint layerCount = 6;

		TextureCube(Core& core);
		virtual ~TextureCube() override;

		/**
		 * Creates the texture on the GPU using the provided settings. The texture is cleared to 'ClearColor'.
		 * The Vulkan image usage flags are derived from texture usage.
		 * @param descriptor texture description.
		 * @param generateMipMaps if mip-maps are auto generated
		 * @param clearColor the color to clear the texture with.
		 * @param requiredFlags image usage flags that are required, 0 = no additional usage flags.
		 * @param errorState contains the error if the texture can't be initialized.
		 * @return if the texture initialized successfully.
		 */
		bool init(const SurfaceDescriptor& descriptor, bool generateMipMaps, const glm::vec4& clearColor, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState);

		/**
		 * @return size of the texture in texels.
		 */
		const glm::vec2 getSize() const							{ return { getWidth(), getHeight() }; }

		/**
		 *	@return width of the texture in texels
		 */
		int getWidth() const									{ return mDescriptor.mWidth; }

		/**
		 *	@return height of the texture in texels
		 */
		int getHeight() const									{ return mDescriptor.mHeight; }

		/**
		 *	@return the vulkan image usage flags
		 */
		VkImageUsageFlags getImageUsageFlags() const			{ return mImageUsageFlags; }

		/**
		 * @return the number of texture layers
		 */
		virtual uint getLayerCount() const override				{ return layerCount; }

		/**
		 * @return the number of texture mip-map levels
		 */
		virtual uint getMipLevels() const override				{ return mMipLevels; }

		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual const ImageData& getHandle() const override		{ return mImageData; }

		/**
		 * Note: This function should actually be protected but is required by `nap::CubeRenderTarget` in the naprenderadvanced module.
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual ImageData& getHandle() override					{ return mImageData; }

		const EUsage						mUsage = EUsage::Static;					///< Texture usage (cube maps are currently always static)

	protected:
		ImageData							mImageData = { TextureCube::layerCount };	///< Cube Texture vulkan image buffers
		uint32								mMipLevels = 1;								///< Total number of generated mip-maps

	private:
		VkImageUsageFlags					mImageUsageFlags = 0;
	};
}
