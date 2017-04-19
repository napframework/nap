// Local Includes
#include "rendertargetresource.h"

namespace nap
{

	bool TextureRenderTargetResource2D::init(InitResult& initResult)
	{
		if (!initResult.check(mColorTexture.getTarget()!= nullptr, "Unable to create render target %s. Color textures not set.", mID.getValue().c_str()))
			return false;

		if (!initResult.check(mDepthTexture.getTarget() != nullptr, "Unable to create render target %s. Depth texture not set.", mID.getValue().c_str()))
			return false;

		mTextureRenderTarget.init((opengl::Texture2D&)mColorTexture.getTarget<MemoryTextureResource2D>()->getTexture(), (opengl::Texture2D&)mDepthTexture.getTarget<MemoryTextureResource2D>()->getTexture());
		if (!initResult.check(mTextureRenderTarget.isValid(), "unable to validate frame buffer: %s", mID.getValue().c_str()))
			return false;

		return true;
	}

	opengl::TextureRenderTarget2D& TextureRenderTargetResource2D::getTarget()
	{
		return mTextureRenderTarget;
	}


	// Resource path is display name for frame buffer
	const std::string TextureRenderTargetResource2D::getDisplayName() const
	{
		return mID.getValue();
	}

} // nap

RTTI_DEFINE(nap::TextureRenderTargetResource2D)
