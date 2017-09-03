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
	* Base class for texture resources
	*/
	class NAPAPI Texture : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
		* Virtual override to be implemented by derived classes
		* @return an opengl texture object
		*/
		virtual const opengl::BaseTexture& getTexture() const = 0;

		/**
		* Non const accessors
		* @return an opengl texture object
		*/
		opengl::BaseTexture& getTexture();

		/**
		* Virtual override to get the resolution of the texture, to be implemented by derived classes
		* @return the resolution of the texture
		*/
		virtual const glm::vec2 getSize() const = 0;

		/**
		* Binds the texture
		* @return if the texture was bound successfully
		*/
		virtual bool bind();

		/**
		* Unbinds the texture
		* @return if the texture wan unbound successfully
		*/
		virtual bool unbind();

		/**
		*	Holds all the texture related parameters
		*/
		nap::TextureParameters mParameters;
	};


	/**
	* 2D Texture resource that only has an in-memory representation.
	*/
	class NAPAPI MemoryTexture2D : public Texture
	{
		RTTI_ENABLE(Texture)
	public:
		using Texture::getTexture;

		/**
		* Creates internal texture resource.
		* @return if the texture was created successfully
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return the 2D texture object
		*/
		virtual const opengl::BaseTexture& getTexture() const override;

		/**
		* @return the resolution of the texture
		*/
		virtual const glm::vec2 getSize() const override;

	public:
		opengl::Texture2DSettings mSettings;

	private:
		std::unique_ptr<opengl::Texture2D> mTexture;				// Texture as created during init
		std::string mDisplayName = "MemoryTexture2D";				// Custom display name
	};
}
