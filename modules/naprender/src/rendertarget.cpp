// Local Includes
#include "rendertarget.h"
#include "ntexturerendertarget2d.h"

RTTI_BEGIN_ENUM(nap::ERenderTargetFormat)
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::R8,		"R8"),	
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::Depth,	"Depth")
RTTI_END_ENUM


RTTI_BEGIN_CLASS(nap::RenderTarget)
	RTTI_PROPERTY("mColorTexture",	&nap::RenderTarget::mColorTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mDepthTexture",	&nap::RenderTarget::mDepthTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mClearColor",	&nap::RenderTarget::mClearColor,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool RenderTarget::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mColorTexture != nullptr, "Unable to create render target %s. Color textures not set.", mID.c_str()))
			return false;

		if (!errorState.check(mDepthTexture != nullptr, "Unable to create render target %s. Depth texture not set.", mID.c_str()))
			return false;

		mTextureRenderTarget = std::make_unique<opengl::TextureRenderTarget2D>();
		if (!errorState.check(mTextureRenderTarget->init(mColorTexture->mTexture, mDepthTexture->mTexture, errorState), "Unable to allocate texture render target"))
			return false;
		mTextureRenderTarget->setClearColor(mClearColor);

		return true;
	}


	bool RenderTarget::switchColorTexture(Texture2D& colorTexture, utility::ErrorState& error)
	{
		if (mTextureRenderTarget->switchColorTexture(colorTexture.mTexture, error))
		{
			mColorTexture = &colorTexture;
			return true;
		}
		return false;
	}


	bool RenderTarget::switchDepthTexture(Texture2D& depthTexture, utility::ErrorState& error)
	{
		if (mTextureRenderTarget->switchDepthTexture(depthTexture.mTexture, error))
		{
			mDepthTexture = &depthTexture;
			return true;
		}
		return false;
	}


	opengl::TextureRenderTarget2D& RenderTarget::getTarget()
	{
		assert(mTextureRenderTarget != nullptr);
		return *mTextureRenderTarget;
	}

} // nap