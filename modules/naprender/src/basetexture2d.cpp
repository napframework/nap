#include "basetexture2d.h"
#include "pixmap.h"

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
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToEdge,	"ClampToEdge"),
	RTTI_ENUM_VALUE(nap::EWrapMode::ClampToBorder,	"ClampToBorder")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::TextureParameters)
	RTTI_PROPERTY("MinFilter",			&nap::TextureParameters::mMinFilter,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxFilter",			&nap::TextureParameters::mMaxFilter,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapVertical",		&nap::TextureParameters::mWrapVertical,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WrapHorizontal",		&nap::TextureParameters::mWrapHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxLodLevel",		&nap::TextureParameters::mMaxLodLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseTexture2D)
	RTTI_PROPERTY("Parameters", 	&nap::BaseTexture2D::mParameters,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", 			&nap::BaseTexture2D::mUsage,		nap::rtti::EPropertyMetaData::Default)
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


static void convertTextureParameters(const nap::TextureParameters& input, opengl::TextureParameters& output)
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

	void BaseTexture2D::init(const opengl::Texture2DSettings& settings)
	{
		// Create the texture with the associated settings
		opengl::TextureParameters gl_params;
		convertTextureParameters(mParameters, gl_params);
		mTexture.init(settings, gl_params, mUsage);
	}

	const glm::vec2 BaseTexture2D::getSize() const
	{
		return glm::vec2(mTexture.getSettings().mWidth, mTexture.getSettings().mHeight);
	}


	int BaseTexture2D::getWidth() const
	{
		return static_cast<int>(mTexture.getSettings().mWidth);
	}


	int BaseTexture2D::getHeight() const
	{
		return static_cast<int>(mTexture.getSettings().mHeight);
	}


	void BaseTexture2D::getData(Pixmap& pixmap)
	{
		if (pixmap.empty())
			pixmap.initFromTexture(*this);
		mTexture.getData(pixmap.getBitmap());
	}


	void BaseTexture2D::startGetData()
	{
		mTexture.asyncStartGetData();
	}


	void BaseTexture2D::endGetData(Pixmap& pixmap)
	{
		if (pixmap.empty())
			pixmap.initFromTexture(*this);
		mTexture.asyncEndGetData(pixmap.getBitmap());
	}


	void BaseTexture2D::bind()
	{
		mTexture.bind();
	}


	void BaseTexture2D::unbind()
	{
		mTexture.unbind();
	}

}