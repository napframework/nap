// Local Includes
#include "rendertarget.h"
#include "texture2d.h"
#include "nrendertarget.h"

RTTI_BEGIN_CLASS(nap::TextureRenderTarget2D)
	RTTI_PROPERTY("mColorTexture",	&nap::TextureRenderTarget2D::mColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mDepthTexture",	&nap::TextureRenderTarget2D::mDepthTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mClearColor",	&nap::TextureRenderTarget2D::mClearColor,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	bool TextureRenderTarget2D::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mColorTexture != nullptr, "Unable to create render target %s. Color textures not set.", mID.c_str()))
			return false;

		if (!errorState.check(mDepthTexture != nullptr, "Unable to create render target %s. Depth texture not set.", mID.c_str()))
			return false;

		mTextureRenderTarget = new opengl::TextureRenderTarget2D;

		mTextureRenderTarget->init((opengl::Texture2D&)mColorTexture->getTexture(), (opengl::Texture2D&)mDepthTexture->getTexture(), mClearColor);
		if (!errorState.check(mTextureRenderTarget->isValid(), "unable to validate frame buffer: %s", mID.c_str()))
			return false;
		
		return true;
	}


	opengl::TextureRenderTarget2D& TextureRenderTarget2D::getTarget()
	{
		assert(mTextureRenderTarget != nullptr);
		return *mTextureRenderTarget;
	}

} // nap
