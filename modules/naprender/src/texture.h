#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <ntexture.h>
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
		EFilterMode mMinFilter		= EFilterMode::LinearMipmapLinear;
		EFilterMode mMaxFilter		= EFilterMode::Linear;
		EWrapMode	mWrapVertical	= EWrapMode::ClampToEdge;
		EWrapMode	mWrapHorizontal = EWrapMode::ClampToEdge;
		int			mMaxLodLevel	= 20;
	};

	/**
	 * Converts texture parameters to an opengl compatible set of parameters
	 * @param input the parameters to convert
	 * @param output the opengl parameters to populate
	 */
	extern void convertTextureParameters(const nap::TextureParameters& input, opengl::TextureParameters& output);

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for all 2-dimensional textures. This class is only intended as base class,
	 * derived implementations should implement the virtual RTTIObject::init() and call the non-virtual Texture2D::init() to
	 * properly initialize the texture. 
	 */
	class NAPAPI Texture2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
	 	 * Initializes opengl texture using the parameters from RTTI and @settings.
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
		 * Activates this texture for rendering.
		 */
		void bind();

		/**
		 * Deactivates this texture for rendering.
		 */
		void unbind();

		nap::TextureParameters		mParameters;		// RTTI texture parameters

	protected:
		virtual ~Texture2D() = default;					// Protected to avoid using this class as-is (it is intended as base class)

	private:
		opengl::Texture2D			mTexture;			// Internal opengl texture
	};

	/**
	 * Memory texture that it is initially empty, that can be set from RTTI.
	 */
	class NAPAPI MemoryTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		enum class EFormat
		{
			RGBA8,			// 4 components, 8 bytes per component
			RGB8,			// 3 components, 8 bytes per component
			R8,				// 1 components, 8 bytes per component
			Depth			// Texture used for binding to depth buffer
		};

		int		mWidth;			// Width of the texture, in texels
		int		mHeight;		// Height of the texture, in texels
		EFormat	mFormat;		// Format of the texture
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
