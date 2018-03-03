#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <ntexture2d.h>
#include <glm/glm.hpp>

namespace nap
{
	class Bitmap;

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

	//////////////////////////////////////////////////////////////////////////

	/**
	 * GPU representation of a 2D bitmap. 
	 * This class does not own any CPU data but offers an interface to up and download texture data from and in to a bitmap
	 */
	class NAPAPI Texture2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
		 * Initializes the opengl texture using the associated parameters and given settings.
		 * @param settings the texture specific settings associated with this texture
		 */
		void initTexture(const opengl::Texture2DSettings& settings);

		/**
		 * Initializes the GPU texture using the settings associated with the incoming bitmap
		 * @param bitmap the cpu pixel data used to initialize this texture with
		 * @param compressed Whether a compressed GPU texture should be created.
		 * @param errorState Contains error information if the function returns false;
		 * @return True on success, false on failure.
		 */
		bool initFromBitmap(const Bitmap& bitmap, bool compressed, utility::ErrorState& errorState);

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

		/**
		 * Activates this texture for rendering.
		 */
		void bind();

		/**
		 * Deactivates this texture for rendering.
		 */
		void unbind();

	public:
		nap::TextureParameters		mParameters;									///< Property: 'Parameters' GPU parameters associated with this texture
		opengl::ETextureUsage		mUsage = opengl::ETextureUsage::Static;			///< Property: 'Usage' How this texture is used, ie: updated on the GPU

	protected:
		opengl::Texture2D& getTexture() { return mTexture; }

	private:
		friend class RenderTarget;
		opengl::Texture2D			mTexture;			///< Internal opengl texture
	};
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
