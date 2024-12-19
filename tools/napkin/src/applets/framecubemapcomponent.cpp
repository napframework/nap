/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "framecubemapcomponent.h"

// External Includes
#include <entity.h>
#include <renderskyboxcomponent.h>
#include <mathutils.h>

// nap::framecubemapcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::FrameCubemapComponent)
	RTTI_PROPERTY("SkyboxComponent",		&napkin::FrameCubemapComponent::mSkyBoxComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitController",		&napkin::FrameCubemapComponent::mOrbitController,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderMeshComponent",	&napkin::FrameCubemapComponent::mRenderMeshComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",		&napkin::FrameCubemapComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Meshes",					&napkin::FrameCubemapComponent::mMeshes,				nap::rtti::EPropertyMetaData::Required)
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


		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		FrameCubemapComponent* resource = getComponent<FrameCubemapComponent>();
		for (auto& mesh : resource->mMeshes)
		{
			RenderableMesh render_mesh = mRenderMeshComponent->createRenderableMesh(*mesh, errorState);
			if (!render_mesh.isValid())
				return false;
			mMeshes.emplace_back(render_mesh);
		}

		// Make sure we have some meshes
		if (!errorState.check(mMeshes.size() > 0, "No mesh files to select from"))
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


	void FrameCubemapComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mRenderMeshComponent->setMesh(mMeshes[mMeshIndex]);
	}
}
