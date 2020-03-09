#include "rendertexture2d.h"

RTTI_BEGIN_ENUM(nap::RenderTexture2D::EFormat)
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGB8,	"RGB8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R8,		"R8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA16,	"RGBA16"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGB16,	"RGB16"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R16,		"R16"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA32,	"RGBA32"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGB32,	"RGB32"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R32,		"R32"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::Depth,	"Depth")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RenderTexture2D)
	RTTI_PROPERTY("Width",	&nap::RenderTexture2D::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::RenderTexture2D::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format", &nap::RenderTexture2D::mFormat, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{
	int RenderTexture2D::getChannelCount()
	{
		switch (mFormat)
		{
			case RenderTexture2D::EFormat::R8:
			case RenderTexture2D::EFormat::R16:
			case RenderTexture2D::EFormat::R32:
			case RenderTexture2D::EFormat::Depth:
				return 1;
			case RenderTexture2D::EFormat::RGB8:
			case RenderTexture2D::EFormat::RGB16:
			case RenderTexture2D::EFormat::RGB32:
				return 3;
			case RenderTexture2D::EFormat::RGBA8:
			case RenderTexture2D::EFormat::RGBA16:
			case RenderTexture2D::EFormat::RGBA32:
				return 4;
			default:
				assert(false);
				return 0;
		}
	}


	// Initializes 2D texture. 
	bool RenderTexture2D::init(utility::ErrorState& errorState)
	{
		opengl::Texture2DSettings settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;

		switch (mFormat)
		{
		case EFormat::RGBA8:
			settings.mFormat			= GL_RGBA;
			settings.mInternalFormat 	= GL_RGBA8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case EFormat::RGB8:
			settings.mFormat			= GL_RGB;
			settings.mInternalFormat 	= GL_RGB8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case EFormat::R8:
			settings.mFormat			= GL_RED;
			settings.mInternalFormat 	= GL_R8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case EFormat::RGBA16:
			settings.mFormat			= GL_RGBA;
			settings.mInternalFormat	= GL_RGBA16;
			settings.mType				= GL_UNSIGNED_SHORT;
			break;
		case EFormat::RGB16:
			settings.mFormat			= GL_RGB;
			settings.mInternalFormat	= GL_RGB16;
			settings.mType				= GL_UNSIGNED_SHORT;
			break;
		case EFormat::R16:
			settings.mFormat			= GL_R;
			settings.mInternalFormat	= GL_R16;
			settings.mType				= GL_UNSIGNED_SHORT;
			break;
		case EFormat::RGBA32:
			settings.mFormat			= GL_RGBA;
			settings.mInternalFormat	= GL_RGBA32F;
			settings.mType				= GL_FLOAT;
			break;
		case EFormat::RGB32:
			settings.mFormat			= GL_RGB;
			settings.mInternalFormat	= GL_RGB32F;
			settings.mType				= GL_FLOAT;
			break;
		case EFormat::R32:
			settings.mFormat			= GL_R;
			settings.mInternalFormat	= GL_R32F;
			settings.mType				= GL_FLOAT;
			break;
		case EFormat::Depth:
			settings.mFormat			= GL_DEPTH_COMPONENT;
			settings.mInternalFormat 	= GL_DEPTH_COMPONENT;
			settings.mType				= GL_FLOAT;
			break;
		}

		initTexture(settings);
		return true;
	}

}
