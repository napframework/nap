/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "frame2dtexturecomponent.h"

// External Includes
#include <entity.h>
#include <textureshader.h>

// nap::appletcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::Frame2DTextureComponent)
	RTTI_PROPERTY("ZoomPanController",		&napkin::Frame2DTextureComponent::mZoomPanController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneTransform",			&napkin::Frame2DTextureComponent::mPlaneTransform,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneRenderer",			&napkin::Frame2DTextureComponent::mPlaneRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRenderer",			&napkin::Frame2DTextureComponent::mMeshRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRotateComponent",	&napkin::Frame2DTextureComponent::mRotateComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",		&napkin::Frame2DTextureComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Meshes",					&napkin::Frame2DTextureComponent::mMeshes,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::appletcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::Frame2DTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	void Frame2DTextureComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
	}

	bool Frame2DTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		//////////////////////////////////////////////////////////////////////////
		// Orthographic Plane
		//////////////////////////////////////////////////////////////////////////

		// 2D texture sampler input
		auto& plane_mat_instance = mPlaneRenderer->getMaterialInstance();
		mPlaneSampler = plane_mat_instance.getOrCreateSampler<Sampler2DInstance>(uniform::texture::sampler::colorTexture);
		if (!errorState.check(mPlaneSampler != nullptr, "Missing 2D texture sampler input '%s'",
			nap::uniform::texture::sampler::colorTexture))
			return false;

		// UBO
		auto* ubo_instance = plane_mat_instance.getOrCreateUniform(uniform::texture::uboStruct);
		if (!errorState.check(ubo_instance != nullptr, "Missing 2D texture uniform '%s'", uniform::texture::uboStruct))
			return false;

		// Opacity
		mPlaneOpacity = ubo_instance->getOrCreateUniform<UniformFloatInstance>(uniform::texture::alpha);
		if (!errorState.check(mPlaneOpacity != nullptr, "Missing 2D texture uniform '%s'", uniform::texture::alpha))
			return false;

		//////////////////////////////////////////////////////////////////////////
		// Mesh
		//////////////////////////////////////////////////////////////////////////

		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		Frame2DTextureComponent* resource = getComponent<Frame2DTextureComponent>();
		for (auto& mesh : resource->mMeshes)
		{
			RenderableMesh render_mesh = mMeshRenderer->createRenderableMesh(*mesh, errorState);
			if (!render_mesh.isValid())
				return false;
			mMeshes.emplace_back(render_mesh);
		}

		// Make sure we have some meshes and set it
		if (!errorState.check(mMeshes.size() > 0, "No mesh files to select from"))
			return false;
		setMeshIndex(0);

		// 2D mesh sampler input
		auto& mesh_mat_instance = mMeshRenderer->getMaterialInstance();
		mMeshSampler = mesh_mat_instance.getOrCreateSampler<Sampler2DInstance>(uniform::texture::sampler::colorTexture);
		if (!errorState.check(mMeshSampler != nullptr, "Missing 2D texture sampler input '%s'",
			nap::uniform::texture::sampler::colorTexture))
			return false;

		// UBO
		ubo_instance = mesh_mat_instance.getOrCreateUniform(uniform::texture::uboStruct);
		if (!errorState.check(ubo_instance != nullptr, "Missing 2D texture uniform '%s'", uniform::texture::uboStruct))
			return false;

		// Opacity
		mMeshOpacity = ubo_instance->getOrCreateUniform<UniformFloatInstance>(uniform::texture::alpha);
		if (!errorState.check(mMeshOpacity != nullptr, "Missing 2D texture uniform '%s'", uniform::texture::alpha))
			return false;

		//////////////////////////////////////////////////////////////////////////

		// Texture to use when selection is cleared
		mTextureFallback = getComponent<Frame2DTextureComponent>()->mFallbackTexture.get();

		// Bind fall-back texture
		bind(*mTextureFallback);

		return true;
	}


	void Frame2DTextureComponentInstance::bind(Texture2D& texture)
	{
		// Move texture into ours
		mSelectedTexture = &texture;

		// Bind texture to plane and mesh shader
		assert(mPlaneSampler != nullptr);
		mPlaneSampler->setTexture(*mSelectedTexture);
		assert(mMeshSampler != nullptr);
		mMeshSampler->setTexture(*mSelectedTexture);
	}


	void Frame2DTextureComponentInstance::frame()
	{
		assert(mPlaneSampler->hasTexture());
		mZoomPanController->frameTexture(mPlaneSampler->getTexture(), *mPlaneTransform);
		mPlaneOpacity->setValue(1.0f);
		mMeshOpacity->setValue(1.0f);
	}


	void Frame2DTextureComponentInstance::clear()
	{
		bind(*mTextureFallback);
	}


	void Frame2DTextureComponentInstance::setOpacity(float opacity)
	{
		switch (mMode)
		{
		case EMode::Plane:
			assert(mPlaneOpacity != nullptr);
			mPlaneOpacity->setValue(opacity);
			break;
		case EMode::Mesh:
			assert(mMeshOpacity != nullptr);
			mMeshOpacity->setValue(opacity);
			break;
		default:
			assert(false);
			break;
		}
	}


	float Frame2DTextureComponentInstance::getOpacity() const
	{
		switch (mMode)
		{
		case EMode::Plane:
			assert(mPlaneOpacity != nullptr);
			return mPlaneOpacity->getValue();
			break;
		case EMode::Mesh:
			assert(mMeshOpacity != nullptr);
			return mMeshOpacity->getValue();
		default:
			assert(false);
			break;
		}
		return 0.0f;
	}


	void Frame2DTextureComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mMeshRenderer->setMesh(mMeshes[mMeshIndex]);
	}
}
