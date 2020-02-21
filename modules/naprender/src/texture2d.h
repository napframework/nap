#pragma once

// External Includes
#include <nap/resource.h>
#include <utility/dllexport.h>
#include <ntexture2d.h>
#include <glm/glm.hpp>
#include <nap/numeric.h>
#include "rtti/factory.h"
#include "vulkan/vulkan_core.h"
#include "vk_mem_alloc.h"
#include "nap/signalslot.h"
#include "surfaceformats.h"

namespace nap
{
	class Bitmap;
	class RenderService;

	/**
	 *	Texture min filter
	 */
	enum class EFilterMode : int
	{
		Nearest = 0,				///< Nearest
		Linear,						///< Linear
		NearestMipmapNearest,		///< NearestMipmapNearest
		LinearMipmapNearest,		///< LinearMipmapNearest
		NearestMipmapLinear,		///< NearestMipmapLinear
		LinearMipmapLinear			///< LinearMipmapLinear
	};

	/**
	 *	Texture wrap mode
	 */
	enum class EWrapMode : int
	{
		Repeat = 0,					///< Repeat 
		MirroredRepeat,				///< MirroredRepeat
		ClampToEdge,				///< ClampToEdge
		ClampToBorder				///< ClampToBorder
	};

	/**
	 * Parameters associated with a texture
	 */
	struct NAPAPI TextureParameters
	{
		EFilterMode mMinFilter = EFilterMode::LinearMipmapLinear;		///< Property: 'MinFilter' minimizing filter
		EFilterMode mMaxFilter = EFilterMode::Linear;					///< Property: 'MaxFilter' maximizing filter	
		EWrapMode	mWrapVertical = EWrapMode::ClampToEdge;				///< Property: 'WrapVertical' vertical wrap mode
		EWrapMode	mWrapHorizontal = EWrapMode::ClampToEdge;			///< Property: 'WarpHorizontal'	horizontal wrap mode
		int			mMaxLodLevel = 20;									///< Property: 'MaxLodLevel' max number of supported lods, 0 = only highest lod
	};

	/**
	* Texture2Dsettings
	*
	* Data associated with a 2d texture
	*/
	struct Texture2DSettings
	{
	public:
		uint32_t			mWidth	  = 0;					//< Specifies the width of the texture
		uint32_t			mHeight	  = 0;					//< Specifies the height of the texture
		ESurfaceDataType	mDataType = nap::ESurfaceDataType::BYTE;
		ESurfaceChannels	mChannels = nap::ESurfaceChannels::BGR;

		bool isValid() const { return mWidth != 0 && mHeight != 0; }
		bool operator==(const Texture2DSettings& other) const { return mWidth == other.mWidth && mHeight == other.mHeight && mDataType == other.mDataType && mChannels == other.mChannels; }
		bool operator!=(const Texture2DSettings& other) const { return !(*this == other); }
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * GPU representation of a 2D bitmap. This is the base class for all 2D textures.
	 * Classes should derive from Texture2D and call either initTexture (for a GPU-only texture) or initFromBitmap (for a CPU & GPU texture).
	 * This class does not own any CPU data but offers an interface to up and download texture data from and in to a bitmap
	 */
	class NAPAPI Texture2D : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		Texture2D() = default;
		Texture2D(RenderService& renderService);

		/**
		 * Initializes the opengl texture using the associated parameters and given settings.
		 * @param settings the texture specific settings associated with this texture
		 */
		bool initTexture(const Texture2DSettings& settings, utility::ErrorState& errorState);

		/**
		 * Initializes the GPU texture using the settings associated with the incoming bitmap
		 * @param bitmap the cpu pixel data used to initialize this texture with
		 * @param compressed Whether a compressed GPU texture should be created.
		 * @param errorState Contains error information if the function returns false;
		 * @return True on success, false on failure.
		 */
		bool initFromBitmap(const Bitmap& bitmap, bool compressed, utility::ErrorState& errorState);

		/**
		 * @return the Texture2D parameters that describe, clamping, interpolation etc.
		 */
		const nap::TextureParameters& geParameters() const { return mParameters; }

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
		 * Converts the CPU data from the Bitmap that is passed in to the GPU. The internal Bitmap remains untouched. 
		 * The bitmap should contain valid data and not be empty.
		 * @param bitmap CPU data to convert to GPU.
		 */
		void update(const Bitmap& bitmap);

		void upload(VkCommandBuffer commandBuffer);

		/**
		 * Converts the CPU data that is passed in to the GPU. The internal Bitmap remains untouched. 
		 * @param data Pointer to the CPU data.
		 * @param pitch Length of a row of bytes in the input data.
		 */
		void update(const void* data, int pitch = 0);

		/**
		 * Blocking call to retrieve GPU texture data that is stored in this texture
		 * When the internal bitmap is empty it will be initialized based on the settings associated with this texture
		 * This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		 * @return reference to the internal bitmap that is filled with the GPU data of this texture.
		 */
		void getData(Bitmap& bitmap);

		/**
		 * @return the OpenGL texture hardware handle.
		 */
		nap::uint getHandle() const;

		/**
		 * Starts a transfer of texture data from GPU to CPU. This is a non blocking call.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered.
		 */
		void startGetData();

		/**
		* Finishes a transfer of texture data from GPU to CPU that was started with startGetData. See comment in startGetData for proper use.
		* When the internal bitmap is empty it will be initialized based on the settings associated with this texture.
		* This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		* @return reference to the internal bitmap that is filled with the GPU data of this texture.
		*/
		void endGetData(Bitmap& bitmap);

		VkImageView getImageView() const;

		nap::Signal<const Texture2D&> changed;

	public:
		nap::TextureParameters		mParameters;									///< Property: 'Parameters' GPU parameters associated with this texture
		opengl::ETextureUsage		mUsage = opengl::ETextureUsage::Static;			///< Property: 'Usage' How this texture is used, ie: updated on the GPU

	protected:
		/**
		 *	@return the opengl texture
		 */
		opengl::Texture2D& getTexture()												{ return mTexture; }

		/**
		 *	@return the opengl texture
		 */
		const opengl::Texture2D& getTexture() const									{ return mTexture; }

	private:
		friend class RenderTarget;

		struct StagingBuffer
		{
			VkBuffer					mStagingBuffer;
			VmaAllocation				mStagingBufferAllocation;
			VmaAllocationInfo			mStagingBufferAllocationInfo;
		};

		struct ImageData
		{
			VkImage						mTextureImage = nullptr;
			VkImageView					mTextureView;
			VmaAllocation				mTextureAllocation = nullptr;
			VmaAllocationInfo			mTextureAllocationInfo;
			VkImageLayout				mCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

		using ImageDataList = std::vector<ImageData>;
		using StagingBufferList = std::vector<StagingBuffer>;

		RenderService*				mRenderService = nullptr;
		opengl::Texture2D			mTexture;
		std::vector<uint8_t>		mTextureData;
		ImageDataList				mImageData;
		StagingBufferList			mStagingBuffers;
		int							mCurrentStagingBufferIndex = -1;
		int							mCurrentImageIndex = -1;
		glm::ivec2					mImageSize;
	};

	using Texture2DCreator = rtti::ObjectCreator<Texture2D, RenderService>;
}

//////////////////////////////////////////////////////////////////////////
// Template Specialization of the Texture filter and wrap modes
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template<>
	struct hash<nap::EFilterMode>
	{
		size_t operator()(const nap::EFilterMode &v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};

	template<>
	struct hash<nap::EWrapMode>
	{
		size_t operator()(const nap::EWrapMode &v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};
}
