#include "texture.h"

RTTI_BEGIN_ENUM(nap::EFilterMode)
	RTTI_ENUM_VALUE(nap::EFilterMode::Nearest, "Nearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::Linear, "Linear"),
	RTTI_ENUM_VALUE(nap::EFilterMode::NearestMipmapNearest, "NearestMipmapNearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::LinearMipmapNearest, "LinearMipmapNearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::NearestMipmapLinear, "NearestMipmapLinear"),
	RTTI_ENUM_VALUE(nap::EFilterMode::LinearMipmapLinear, "LinearMipmapLinear")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EWrapMode)
	RTTI_ENUM_VALUE(nap::EWrapMode::Repeat,			"Repeat"),
	RTTI_ENUM_VALUE(nap::EWrapMode::MirroredRepeat, "MirroredRepeat"),
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToBorder,	"ClampToEdge"),
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToBorder,	"ClampToBorder")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::MemoryTexture2D::EFormat)
	RTTI_ENUM_VALUE(nap::MemoryTexture2D::EFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::MemoryTexture2D::EFormat::RGB8,	"RGB8"),
	RTTI_ENUM_VALUE(nap::MemoryTexture2D::EFormat::Depth,	"Depth")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::TextureParameters)
	RTTI_PROPERTY("MinFilter",			&nap::TextureParameters::mMinFilter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxFilter",			&nap::TextureParameters::mMaxFilter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WrapVertical",		&nap::TextureParameters::mWrapVertical,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WrapHorizontal",		&nap::TextureParameters::mWrapHorizontal,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxLodLevel",		&nap::TextureParameters::mMaxLodLevel,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Texture2D)
	RTTI_PROPERTY("mParameters", 		&nap::Texture2D::mParameters,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MemoryTexture2D)
	RTTI_PROPERTY("Width",	&nap::MemoryTexture2D::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::MemoryTexture2D::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format", &nap::MemoryTexture2D::mFormat, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
* openglFilterMap
*
* Maps Filter modes to supported GL formats
*/
using OpenglFilterMap = std::unordered_map<nap::EFilterMode, GLint>;
static const OpenglFilterMap openglFilterMap =
{
	{ nap::EFilterMode::Nearest,					GL_NEAREST},
	{ nap::EFilterMode::Linear,						GL_LINEAR },
	{ nap::EFilterMode::NearestMipmapNearest,		GL_NEAREST_MIPMAP_NEAREST },
	{ nap::EFilterMode::LinearMipmapNearest,		GL_LINEAR_MIPMAP_NEAREST },
	{ nap::EFilterMode::NearestMipmapLinear,		GL_NEAREST_MIPMAP_LINEAR },
	{ nap::EFilterMode::LinearMipmapLinear,			GL_LINEAR_MIPMAP_LINEAR}
};


/**
 *	openglWrapMap
 *
 * Maps Wrap modes to supported GL formats
 */
using OpenglWrapMap = std::unordered_map<nap::EWrapMode, GLint>;
static const OpenglWrapMap openglWrapMap =
{	 
	{ nap::EWrapMode::Repeat,						GL_REPEAT },
	{ nap::EWrapMode::MirroredRepeat,				GL_MIRRORED_REPEAT },
	{ nap::EWrapMode::ClampToEdge,					GL_CLAMP_TO_EDGE },
	{ nap::EWrapMode::ClampToBorder,				GL_CLAMP_TO_BORDER }
};


/**
 *	@return the opengl filter based on @filter
 */
static GLint getGLFilterMode(nap::EFilterMode filter)
{
	auto it = openglFilterMap.find(filter);
	assert(it != openglFilterMap.end());
	return it->second;
}


/**
 *	@return the opengl wrap mode based on @wrapmode
 */
static GLint getGLWrapMode(nap::EWrapMode wrapmode)
{
	auto it = openglWrapMap.find(wrapmode);
	assert(it != openglWrapMap.end());
	return it->second;
}


void nap::convertTextureParameters(const TextureParameters& input, opengl::TextureParameters& output)
{
	output.minFilter	=	getGLFilterMode(input.mMinFilter);
	output.maxFilter	=	getGLFilterMode(input.mMaxFilter);
	output.wrapVertical =	getGLWrapMode(input.mWrapVertical);
	output.wrapHorizontal = getGLWrapMode(input.mWrapHorizontal);
	output.maxLodLevel =	input.mMaxLodLevel;
}


//////////////////////////////////////////////////////////////////////////

namespace nap
{

	void Texture2D::init(opengl::Texture2DSettings& settings)
	{
		// Create the texture with the associated settings
		opengl::TextureParameters gl_params;
		convertTextureParameters(mParameters, gl_params);
		mTexture.setParameters(gl_params);
		mTexture.init(settings);
	}

	const glm::vec2 Texture2D::getSize() const
	{
		return glm::vec2(mTexture.getSettings().width, mTexture.getSettings().height);
	}

	void Texture2D::bind()
	{
		mTexture.bind();
	}

	void Texture2D::unbind()
	{
		mTexture.unbind();
	}

	//////////////////////////////////////////////////////////////////////////


	// Initializes 2D texture. 
	bool MemoryTexture2D::init(utility::ErrorState& errorState)
	{
		opengl::Texture2DSettings settings;
		settings.width = mWidth;
		settings.height = mHeight;

		switch (mFormat)
		{
		case EFormat::RGBA8:
			settings.format			= GL_RGBA;
			settings.internalFormat = GL_RGBA8;
			settings.type			= GL_UNSIGNED_BYTE;
			break;
		case EFormat::RGB8:
			settings.format			= GL_RGB;
			settings.internalFormat = GL_RGB8;
			settings.type			= GL_UNSIGNED_BYTE;
			break;
		case EFormat::Depth:
			settings.format			= GL_DEPTH_COMPONENT;
			settings.internalFormat = GL_DEPTH_COMPONENT;
			settings.type			= GL_FLOAT;
			break;
		}
		
		Texture2D::init(settings);
		return true;
	}

}
