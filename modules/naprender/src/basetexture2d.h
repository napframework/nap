#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <ntexture2d.h>
#include <glm/glm.hpp>

namespace nap
{
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
		* Initializes opengl texture using the parameters from RTTI and @settings.
		*/
		void init(opengl::Texture2DSettings& settings);

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
