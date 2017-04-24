// Local Includes
#include "rendertargetresource.h"

namespace nap
{
	// Return the frame buffer, initialize if necessary
	opengl::TextureRenderTarget2D& TextureRenderTargetResource2D::getTarget()
	{
		// If the render target hasn't been loaded, do so
		if (!mLoaded)
		{
			if (mColorTexture == nullptr || mDepthTexture == nullptr)
			{
				nap::Logger::warn("Unable to create render target %s. Color and/or depth textures missing.", getResourcePath().c_str());
				return mTextureRenderTarget;
			}

			mTextureRenderTarget.init((opengl::Texture2D&)mColorTexture->getTexture(), (opengl::Texture2D&)mDepthTexture->getTexture());
			if (!mTextureRenderTarget.isValid())
			{
				nap::Logger::warn("unable to validate frame buffer: %s", getResourcePath().c_str());
			}
			mLoaded = true;
		}

		return mTextureRenderTarget;
	}


	// Resource path is display name for frame buffer
	const std::string& TextureRenderTargetResource2D::getDisplayName() const
	{
		return getResourcePath();
	}

} // nap

RTTI_DEFINE(nap::TextureRenderTargetResource2D)
