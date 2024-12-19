/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "framecubemapcomponent.h"

// External Includes
#include <entity.h>
#include <renderskyboxcomponent.h>

// nap::framecubemapcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::FrameCubemapComponent)
	RTTI_PROPERTY("SkyboxComponent",		&napkin::FrameCubemapComponent::mSkyBoxComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitController",		&napkin::FrameCubemapComponent::mOrbitController,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderMeshComponent",	&napkin::FrameCubemapComponent::mRenderMeshComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",		&napkin::FrameCubemapComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::framecubemapcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::FrameCubemapComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{

	bool FrameCubemapComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch reflective mesh cubemap sampler input
		mReflectiveCubeSampler = mRenderMeshComponent->getMaterialInstance().getOrCreateSampler<SamplerCubeInstance>("environmentMap");
		if (!errorState.check(mReflectiveCubeSampler != nullptr, "Missing cube sampler input '%s'", "environmentMap"))
			return false;

		// Fetch and bind texture fallback
		mTextureFallback = getComponent<FrameCubemapComponent>()->mFallbackTexture.get();
		bind(*mTextureFallback);

		return true;
	}


	void FrameCubemapComponentInstance::bind(TextureCube& texture)
	{
		mSkyboxComponent->setTexture(texture);
		mReflectiveCubeSampler->setTexture(texture);
	}


	void FrameCubemapComponentInstance::frame()
	{
		mOrbitController->enable({ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 0.0f });
		mSkyboxComponent->setOpacity(1.0f);
	}


	void FrameCubemapComponentInstance::clear()
	{
		
	}
}
