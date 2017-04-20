// Local Includes
#include "rendertargetresource.h"

RTTI_BEGIN_CLASS(nap::TextureRenderTargetResource2D)
	RTTI_PROPERTY_REQUIRED("mColorTexture", &nap::TextureRenderTargetResource2D::mColorTexture)
	RTTI_PROPERTY_REQUIRED("mDepthTexture", &nap::TextureRenderTargetResource2D::mDepthTexture)
RTTI_END_CLASS

namespace nap
{

	bool TextureRenderTargetResource2D::init(InitResult& initResult)
	{
		if (!initResult.check(mColorTexture != nullptr, "Unable to create render target %s. Color textures not set.", mID.c_str()))
			return false;

		if (!initResult.check(mDepthTexture != nullptr, "Unable to create render target %s. Depth texture not set.", mID.c_str()))
			return false;

		mPrevTextureRenderTarget = mTextureRenderTarget;
		mTextureRenderTarget = new opengl::TextureRenderTarget2D;

		mTextureRenderTarget->init((opengl::Texture2D&)mColorTexture->getTexture(), (opengl::Texture2D&)mDepthTexture->getTexture());
		if (!initResult.check(mTextureRenderTarget->isValid(), "unable to validate frame buffer: %s", mID.c_str()))
			return false;

		if (mPrevTextureRenderTarget)
			mTextureRenderTarget->setClearColor(mPrevTextureRenderTarget->getClearColor()); // HACK

		return true;
	}

	void TextureRenderTargetResource2D::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			if (mPrevTextureRenderTarget != nullptr)
			{
				delete mPrevTextureRenderTarget;
				mPrevTextureRenderTarget = nullptr;
			}
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			delete mTextureRenderTarget;
			mTextureRenderTarget = mPrevTextureRenderTarget;
			mPrevTextureRenderTarget = nullptr;
		}
	}


	opengl::TextureRenderTarget2D& TextureRenderTargetResource2D::getTarget()
	{
		assert(mTextureRenderTarget != nullptr);
		return *mTextureRenderTarget;
	}


	// Resource path is display name for frame buffer
	const std::string TextureRenderTargetResource2D::getDisplayName() const
	{
		return mID;
	}

} // nap
