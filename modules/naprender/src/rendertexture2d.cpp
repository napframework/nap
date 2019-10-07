#include "rendertexture2d.h"

RTTI_BEGIN_CLASS(nap::RenderTexture2D)
	RTTI_PROPERTY("Width",	&nap::RenderTexture2D::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::RenderTexture2D::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format", &nap::RenderTexture2D::mFormat, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{
	// Initializes 2D texture. 
	bool RenderTexture2D::init(utility::ErrorState& errorState)
	{
		opengl::Texture2DSettings settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;

		switch (mFormat)
		{
		case ERenderTargetFormat::RGBA8:
			settings.mFormat			= GL_RGBA;
			settings.mInternalFormat 	= GL_RGBA8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case ERenderTargetFormat::RGB8:
			settings.mFormat			= GL_RGB;
			settings.mInternalFormat 	= GL_RGB8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case ERenderTargetFormat::R8:
			settings.mFormat			= GL_RED;
			settings.mInternalFormat 	= GL_R8;
			settings.mType				= GL_UNSIGNED_BYTE;
			break;
		case ERenderTargetFormat::Depth:
			settings.mFormat			= GL_DEPTH_COMPONENT;
			settings.mInternalFormat 	= GL_DEPTH_COMPONENT;
			settings.mType				= GL_FLOAT;
			break;
		}

		initTexture(settings);
		return true;
	}

}
