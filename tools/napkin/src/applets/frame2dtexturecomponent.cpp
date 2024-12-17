#include "frame2dtexturecomponent.h"

// External Includes
#include <entity.h>
#include <textureshader.h>

// nap::appletcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::Frame2DTextureComponent)
	RTTI_PROPERTY("ZoomPanController",	&napkin::Frame2DTextureComponent::mZoomPanController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",	&napkin::Frame2DTextureComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::appletcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::Frame2DTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	void Frame2DTextureComponent::getDependentComponents(std::vector<nap::rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
	}



	bool Frame2DTextureComponentInstance::init(nap::utility::ErrorState& errorState)
	{
		// 2D texture transformation
		mTextureTransform = getEntityInstance()->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mTextureTransform != nullptr, "Missing 2D texture transform component"))
			return false;

		// 2D texture renderer
		mTextureRenderer = getEntityInstance()->findComponent<nap::RenderableMeshComponentInstance>();
		if (!errorState.check(mTextureTransform != nullptr, "Missing 2D texture render component"))
			return false;

		// 2D texture sampler input
		auto& mat_instance = mTextureRenderer->getMaterialInstance();
		mSampler = mat_instance.getOrCreateSampler<nap::Sampler2DInstance>(nap::uniform::texture::sampler::colorTexture);
		if (!errorState.check(mSampler != nullptr, "Missing 2D texture sampler input '%s'",
			nap::uniform::texture::sampler::colorTexture))
			return false;

		// Texture to use when selection is cleared
		mTextureFallback = getComponent<Frame2DTextureComponent>()->mFallbackTexture.get();

		// Bind fallback texture
		bind(*mTextureFallback);

		return true;
	}


	void Frame2DTextureComponentInstance::update(double deltaTime)
	{

	}


	void Frame2DTextureComponentInstance::bind(nap::Texture2D& texture)
	{
		// Move texture into ours
		mSelectedTexture = &texture;

		// Bind texture to shader
		assert(mSampler != nullptr);
		mSampler->setTexture(*mSelectedTexture);
	}


	void Frame2DTextureComponentInstance::frame()
	{
		assert(mSampler->hasTexture());
		mZoomPanController->frameTexture(mSampler->getTexture(), *mTextureTransform);	
	}


	void Frame2DTextureComponentInstance::clear()
	{
		bind(*mTextureFallback);
	}
}

