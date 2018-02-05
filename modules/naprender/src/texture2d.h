#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <ntexture2d.h>
#include <glm/glm.hpp>
#include "bitmap.h"

namespace nap
{
	class Bitmap;

	/**
	 *	Texture min filter
	 */
	enum class NAPAPI EFilterMode : int
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
	enum class NAPAPI EWrapMode : int
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
	 * Represents both CPU and GPU data for a texture. The CPU data is stored internally as a bitmap and is optional.
	 * Classes should derive from Texture2D and call either initTexture (for a GPU-only texture) or initFromBitmap (for a CPU & GPU texture).
	 * GPU textures can be read back to CPU using the getData functions. This will fill the internal bitmap with the data read-back from the GPU.
	 * A texture can be modified in two ways:
	 * - By modifying the internal bitmap (retrieved through getBitmap()) and calling update(). This is the most common way of updating the texture.
	 *   When updating the texture in this way, the formats & size of the CPU and GPU textures are guaranteed to match.
	 * - By calling update directly with a custom data buffer. This is useful if you already have data available and don't want the extra overhead of 
	 *   copying to the internal bitmap first. When updating the texture in this way, you are responsible for making sure that the data buffer you pass in 
	 *   matches the format & size of the GPU texture.
	 */
	class NAPAPI Texture2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
        using rtti::RTTIObject::init;

		/**
		 * Initializes opengl texture using the associated parameters and @settings.
		 * @param settings the texture specific settings associated with this texture
		 */
		void initTexture(const opengl::Texture2DSettings& settings);

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
		 *	@return CPU data for this texture in the form of a Bitmap. The Bitmap can be empty if this is a GPU-only texture.
		 */
		Bitmap& getBitmap() { return mBitmap; }

		/**
		 *	Converts the CPU data in the internal Bitmap to the GPU. The bitmap should contain valid data and not be empty.
		 */
		void update();

		/**
		 * Converts the CPU data from the Bitmap that is passed in to the GPU. The internal Bitmap remains untouched. 
		 * The bitmap should contain valid data and not be empty.
		 * @param bitmap CPU data to convert to GPU.
		 */
		void update(Bitmap& bitmap);

		/**
		 * Converts the CPU data that is passed in to the GPU. The internal Bitmap remains untouched. 
		 * @param data Pointer to the CPU data.
		 * @param pitch Length of a row of bytes in the input data.
		 */
		void update(void* data, int pitch = 0);

		/**
		 * Blocking call to retrieve GPU texture data that is stored in this texture
		 * When the internal bitmap is empty it will be initialized based on the settings associated with this texture
		 * This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		 * @return reference to the internal bitmap that is filled with the GPU data of this texture.
		 */
		Bitmap& getData();

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
		Bitmap& endGetData();

		/**
		 * Activates this texture for rendering.
		 */
		void bind();

		/**
		 * Deactivates this texture for rendering.
		 */
		void unbind();

		nap::TextureParameters		mParameters;							// Property: 'Parameters' RTTI texture parameters
		opengl::ETextureUsage		mUsage = opengl::ETextureUsage::Static;	// Property: 'Usage' The usage of this texture

	protected:
		/**
		 * Initializes the GPU texture from the internal bitmap. The bitmap must be filled in by derived classes before
		 * calling this function.
		 * @compressed Whether a compressed GPU texture should be created.
		 * @errorState Contains error information if the function returns false;
		 * @return True on success, false on failure.
		 */
		bool initFromBitmap(bool compressed, utility::ErrorState& errorState);

	private:
		friend class RenderTarget;
		opengl::Texture2D			mTexture;			///< Internal opengl texture
		Bitmap						mBitmap;			///< The CPU image representation
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
