/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "framecubemapcomponent.h"

// External Includes
#include <entity.h>
#include <renderskyboxcomponent.h>
#include <mathutils.h>
#include <inputrouter.h>
#include <renderglobals.h>
#include <meshutils.h>

// nap::framecubemapcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::FrameCubemapComponent)
	RTTI_PROPERTY("SkyboxComponent",		&napkin::FrameCubemapComponent::mSkyBoxComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",		&napkin::FrameCubemapComponent::mCameraComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OrbitController",		&napkin::FrameCubemapComponent::mOrbitController,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderMeshComponent",	&napkin::FrameCubemapComponent::mRenderMeshComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RotateComponent",		&napkin::FrameCubemapComponent::mRotateComponent,		nap::rtti::EPropertyMetaData::Required)
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
	FrameCubemapComponentInstance::~FrameCubemapComponentInstance()
	{
		// Explicitly destroy resource -> unregisters itself with the service
		// TODO: This should happen automatic when texture is destroyed when manually created
		if (mTexture != nullptr)
			mTexture->onDestroy();
		mTexture.reset(nullptr);
	}


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
			// Create mesh / material combo
			RenderableMesh render_mesh = mRenderMeshComponent->createRenderableMesh(*mesh, errorState);
			if (!render_mesh.isValid())
				return false;

			// Find position attr
			if (!errorState.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::position) != nullptr,
				"%s: missing '%s' (vec3) vertex attribute", mesh->mID.c_str(), vertexid::position))
				return false;

			// Add bounds and meshes
			mBounds.emplace_back(utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance()));
			mMeshes.emplace_back(std::move(render_mesh));
		}

		// Make sure we have some meshes
		if (!errorState.check(mMeshes.size() > 0, "No mesh files to select from"))
			return false;

		// Fetch and bind texture fallback
		mTextureFallback = getComponent<FrameCubemapComponent>()->mFallbackTexture.get();

		// Setup
		setMeshIndex(0);
		bind(*mTextureFallback);

		return true;
	}


	void FrameCubemapComponentInstance::load(std::unique_ptr<TextureCube> texure)
	{
		// Bind new texture
		bind(*texure);

		// Explicitly destroy resource -> unregisters itself with the service
		// TODO: This should happen automatic when texture is destroyed when manually created
		if (mTexture != nullptr)
			mTexture->onDestroy();

		// Replace active texture
		mTexture = std::move(texure);
	}


	bool FrameCubemapComponentInstance::load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error)
	{
		// Catch most obvious explicit error -> missing uv attribute
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::normal) != nullptr,
			"Unable to bind texture, '%s' has no %s vec3 vertex attribute", mesh->mID.c_str(), vertexid::normal))
			return false;

		// Catch most obvious explicit error -> missing pos attribute
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::position) != nullptr,
			"Unable to bind texture, '%s' has no %s vec3 vertex attribute", mesh->mID.c_str(), vertexid::position))
			return false;

		// Try and create a render-able mesh
		RenderableMesh render_mesh = mRenderMeshComponent->createRenderableMesh(*mesh, error);
		if (!render_mesh.isValid())
			return false;

		// Pop and add
		if (hasMeshLoaded())
		{
			mMeshes.pop_back();
			mBounds.pop_back();
		}

		// Add renderable mesh, bbox and cache
 		mMeshes.emplace_back(std::move(render_mesh));
		mBounds.emplace_back(utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance()));
		mMesh = std::move(mesh);

		// Select
		setMeshIndex(mMeshes.size() - 1);
		return true;
	}


	void FrameCubemapComponentInstance::bind(TextureCube& texture)
	{
		mSkyboxComponent->setTexture(texture);
		mReflectiveCubeSampler->setTexture(texture);
	}


	void FrameCubemapComponentInstance::frame()
	{
		const auto& bounds = getBounds();
		auto center = bounds.getCenter();
		glm::vec3 camera = { 0.0f, center.y, bounds.getMax().z + 2.0f };

		mOrbitController->enable(camera, center);
		mSkyboxComponent->setOpacity(1.0f);
		mRotateComponent->reset();
		mRotateComponent->setSpeed(0.0f);
	}


	void FrameCubemapComponentInstance::clear()
	{
		bind(*mTextureFallback);
	}


	const nap::TextureCube& FrameCubemapComponentInstance::getTexture() const
	{
		return mSkyboxComponent->getTexture();
	}


	void FrameCubemapComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mRenderMeshComponent->setMesh(mMeshes[mMeshIndex]);
	}


	const IMesh& FrameCubemapComponentInstance::getMesh()
	{
		assert(mMeshes.size() > mMeshIndex);
		return mMeshes[mMeshIndex].getMesh();
	}


	bool FrameCubemapComponentInstance::hasMeshLoaded() const
	{
		return mMesh != nullptr;
	}


	void FrameCubemapComponentInstance::processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window)
	{
		static DefaultInputRouter input_router;
		inputService.processWindowEvents(window, input_router, { mCameraComponent->getEntityInstance() });
	}


	void FrameCubemapComponentInstance::draw(RenderService& renderService, RenderWindow& window)
	{
		// First draw skybox, then reflective mesh
		renderService.renderObjects(window, *mCameraComponent,  { mSkyboxComponent.get() });
		renderService.renderObjects(window, *mCameraComponent,  { mRenderMeshComponent.get() });
	}
}

