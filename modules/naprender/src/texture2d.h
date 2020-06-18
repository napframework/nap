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
	class Bitmap;
	class RenderService;
	class Core;

	/**
	 * Flag that determines how the texture is used at runtime.
	 */
	enum class ETextureUsage
	{
		Static,				///< Texture does not change
		DynamicRead,		///< Texture is frequently read from GPU to CPU
		DynamicWrite		///< Texture is frequently updated from CPU to GPU
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * GPU representation of a 2D bitmap. This is the base class for all 2D textures.
	 * This class does not own any CPU data but offers an interface to up and download texture data from and in to a bitmap
	 */
	class NAPAPI Texture2D : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		Texture2D(Core& core);
		~Texture2D();

		enum class EClearMode
		{
			FillWithZero,
			DontClear
		};
		
		/**
		 * Initializes the opengl texture using the associated parameters and given settings.
		 * @param settings the texture specific settings associated with this texture
		 */
		bool init(const SurfaceDescriptor& descriptor, EClearMode clearMode, utility::ErrorState& errorState);
		bool init(const SurfaceDescriptor& descriptor, void* initialData, utility::ErrorState& errorState);

		/**
		 * @return size of the texture, in texels.
		 */
		const glm::vec2 getSize() const; 

		/**
		 *	@return width of the texture, in texels
		 */
		int getWidth() const;

		/**
		 *	@return height of the texture, in texels
		 */
		int getHeight() const;

		/**
		 * @return the surface description of this texture
		 */
		const SurfaceDescriptor& getDescriptor() const;

		/**
		 * Converts the CPU data that is passed in to the GPU. The internal Bitmap remains untouched. 
		 * @param data Pointer to the CPU data.
		 * @param pitch Length of a row of bytes in the input data.
		 */
		void update(const void* data, int width, int height, int pitch, ESurfaceChannels channels);

		void update(const void* data, const SurfaceDescriptor& surfaceDescriptor);

		VkFormat getVulkanFormat() const { return mVulkanFormat; }

		/**
		 * Starts a transfer of texture data from GPU to CPU. This is a non blocking call. When the transfer completes, the bitmap will be filled with the texture data.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered.
		 * @a bitmap reference to the bitmap that is filled with the GPU data of this texture when the readback completes.
		 */
		void asyncGetData(Bitmap& bitmap);


		/**
		 * @return Vulkan image view
		 */
		VkImageView getImageView() const { return mImageData.mTextureView; }

		/**
		 * @return render service
		 */
		RenderService& getRenderService()				{ return *mRenderService; }

	private:
		void upload(VkCommandBuffer commandBuffer);
		void notifyDownloadReady(int frameIndex);		
		void download(VkCommandBuffer commandBuffer);

	public:
		ETextureUsage				mUsage = ETextureUsage::Static;					///< Property: 'Usage' How this texture is used, ie: updated on the GPU

	protected:
		RenderService*				mRenderService = nullptr;

	private:
		friend class RenderService;

		using TextureReadCallback = std::function<void(void* data, size_t sizeInBytes)>;

		struct StagingBuffer
		{
			VkBuffer				mStagingBuffer;
			VmaAllocation			mStagingBufferAllocation;
			VmaAllocationInfo		mStagingBufferAllocationInfo;
		};

		using StagingBufferList = std::vector<StagingBuffer>;
		std::vector<uint8_t>		mTextureData;
		ImageData					mImageData;
		StagingBufferList			mStagingBuffers;
		int							mCurrentStagingBufferIndex = -1;
		size_t						mImageSizeInBytes = -1;
		SurfaceDescriptor			mDescriptor;
		VkFormat					mVulkanFormat = VK_FORMAT_UNDEFINED;
		std::vector<TextureReadCallback>	mReadCallbacks;
	};

	VkFormat getTextureFormat(RenderService& renderService, const SurfaceDescriptor& descriptor);
}
