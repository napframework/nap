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
		Nearest = 0,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	/**
	 *	Texture wrap mode
	 */
	enum class NAPAPI EWrapMode : int
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	/**
	 * Parameters associated with a texture
	 */
	struct NAPAPI TextureParameters
	{
		EFilterMode mMinFilter = EFilterMode::LinearMipmapLinear;
		EFilterMode mMaxFilter = EFilterMode::Linear;
		EWrapMode	mWrapVertical = EWrapMode::ClampToEdge;
		EWrapMode	mWrapHorizontal = EWrapMode::ClampToEdge;
		int			mMaxLodLevel = 20;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents both CPU and GPU data for a texture. The CPU data is stored internally as a pixmap and is optional.
	 * Classes should derive from Texture2D and call either initTexture (for a GPU-only texture) or initFromPixmap (for a CPU & GPU texture).
	 * GPU textures can be read back to CPU using the getData functions. This will fill the internal pixmap with the data read-back from the GPU.
	 * A texture can be modified in two ways:
	 * - By modifying the internal pixmap (retrieved through getPixmap()) and calling update(). This is the most common way of updating the texture.
	 *   When updating the texture in this way, the formats & size of the CPU and GPU textures are guaranteed to match.
	 * - By calling update directly with a custom data buffer. This is useful if you already have data available and don't want the extra overhead of 
	 *   copying to the internal pixmap first. When updating the texture in this way, you are responsible for making sure that the data buffer you pass in 
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
		 *	@return CPU data for this texture in the form of a Pixmap. The Pixmap can be empty if this is a GPU-only texture.
		 */
		Bitmap& getPixmap() { return mPixmap; }

		/**
		 *	Converts the CPU data in the internal Pixmap to the GPU. The pixmap should contain valid data and not be empty.
		 */
		void update();

		/**
		 * Converts the CPU data from the Pixmap that is passed in to the GPU. The internal Pixmap remains untouched. 
		 * The pixmap should contain valid data and not be empty.
		 * @param pixmap CPU data to convert to GPU.
		 */
		void update(Bitmap& pixmap);

		/**
		 * Converts the CPU data that is passed in to the GPU. The internal Pixmap remains untouched. 
		 * @param data Pointer to the CPU data.
		 * @param pitch Length of a row of bytes in the input data.
		 */
		void update(void* data, int pitch = 0);

		/**
		 * Blocking call to retrieve GPU texture data that is stored in this texture
		 * When the internal pixmap is empty it will be initialized based on the settings associated with this texture
		 * This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		 * @return reference to the internal pixmap that is filled with the GPU data of this texture.
		 */
		Bitmap& getData();

		/**
		 * Starts a transfer of texture data from GPU to CPU. This is a non blocking call.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered.
		 */
		void startGetData();

		/**
		* Finishes a transfer of texture data from GPU to CPU that was started with startGetData. See comment in startGetData for proper use.
		* When the internal pixmap is empty it will be initialized based on the settings associated with this texture.
		* This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		* @return reference to the internal pixmap that is filled with the GPU data of this texture.
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

		nap::TextureParameters		mParameters;							// RTTI texture parameters
		opengl::ETextureUsage		mUsage = opengl::ETextureUsage::Static;	// The usage of this texture

	protected:
		/**
		 * Initializes the GPU texture from the internal pixmap. The pixmap must be filled in by derived classes before
		 * calling this function.
		 * @compressed Whether a compressed GPU texture should be created.
		 * @errorState Contains error information if the function returns false;
		 * @return True on success, false on failure.
		 */
		bool initFromPixmap(bool compressed, utility::ErrorState& errorState);

	private:
		friend class RenderTarget;
		opengl::Texture2D			mTexture;			///< Internal opengl texture
		Bitmap						mPixmap;			///< The CPU image representation
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
