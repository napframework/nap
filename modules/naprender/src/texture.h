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


	class NAPAPI Texture2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		void init(opengl::Texture2DSettings& settings);

		const opengl::Texture2D& getTexture() const { return mTexture; }
		opengl::Texture2D& getTexture() { return mTexture; }
		const glm::vec2 getSize() const;
		void bind();
		void unbind();

		nap::TextureParameters		mParameters;

	private:
		opengl::Texture2D			mTexture;	// Internal opengl texture
	};

	/**
	* 2D Texture resource that only has an in-memory representation.
	*/
	class NAPAPI MemoryTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		* Creates internal texture resource.
		* @return if the texture was created successfully
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		opengl::Texture2DSettings	mSettings;
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
