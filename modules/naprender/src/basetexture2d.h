#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <ntexture2d.h>
#include <glm/glm.hpp>

namespace nap
{
	class Pixmap;

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
	 * Base class for all 2-dimensional textures. This class is only intended as base class,
	 * derived implementations should implement the virtual RTTIObject::init() and call the non-virtual Texture2D::init() to
	 * properly initialize the texture.
	 */
	class NAPAPI BaseTexture2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
        using rtti::RTTIObject::init;

		/**
		 * Initializes opengl texture using the associated parameters and @settings.
		 * @param settings the texture specific settings associated with this texture
		 */
		void init(opengl::Texture2DSettings& settings);

		/**
		 * @return OpenGL Texture2D.
		 */
		const opengl::Texture2D& getTexture() const { return mTexture; }

		/**
		* @return OpenGL Texture2D.
		*/
		opengl::Texture2D& getTexture() { return mTexture; }

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
		 * Blocking call to retrieve GPU texture data that is stored in this texture
		 * When the pixmap is empty it will be initialized based on the settings associated with this texture
		 * This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match
		 * @param pixmap the pixmap that is filled with the data in this texture
		 */
		void getData(Pixmap& pixmap);

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

	private:
		opengl::Texture2D			mTexture;			// Internal opengl texture
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
