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
	RTTI_PROPERTY("SkyboxRenderer",			&napkin::FrameCubemapComponent::mSkyRenderer,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SkboxTransform",			&napkin::FrameCubemapComponent::mSkyTransform,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",		&napkin::FrameCubemapComponent::mCameraComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshOrbit",				&napkin::FrameCubemapComponent::mMeshOrbit,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRenderer",			&napkin::FrameCubemapComponent::mMeshRenderer,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRotate",				&napkin::FrameCubemapComponent::mMeshRotate,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshTransform",			&napkin::FrameCubemapComponent::mMeshTransform,			nap::rtti::EPropertyMetaData::Required)
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
		mReflectiveCubeSampler = mMeshRenderer->getMaterialInstance().getOrCreateSampler<SamplerCubeInstance>("environmentMap");
		if (!errorState.check(mReflectiveCubeSampler != nullptr, "Missing cube sampler input '%s'", "environmentMap"))
			return false;


		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		FrameCubemapComponent* resource = getComponent<FrameCubemapComponent>();
		for (auto& mesh : resource->mMeshes)
		{
			// Create mesh / material combo
			RenderableMesh render_mesh = mMeshRenderer->createRenderableMesh(*mesh, errorState);
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

		// Fetch normalized rotation speed
		mSpeedReference = mMeshOrbit->getMovementSpeed();

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


	int FrameCubemapComponentInstance::load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error)
	{
		// Catch most obvious explicit error -> missing uv attribute
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::normal) != nullptr,
			"Unable to bind texture, '%s' has no %s vec3 vertex attribute", mesh->mID.c_str(), vertexid::normal))
			return -1;

		// Catch most obvious explicit error -> missing pos attribute
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::position) != nullptr,
			"Unable to bind texture, '%s' has no %s vec3 vertex attribute", mesh->mID.c_str(), vertexid::position))
			return -1;

		// Try and create a render-able mesh
		RenderableMesh render_mesh = mMeshRenderer->createRenderableMesh(*mesh, error);
		if (!render_mesh.isValid())
			return -1;

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

		// Bind item for rendering if selected
		setMeshIndex(getMeshIndex());
		return mMeshes.size() - 1;
	}


	void FrameCubemapComponentInstance::bind(TextureCube& texture)
	{
		mSkyRenderer->setTexture(texture);
		mReflectiveCubeSampler->setTexture(texture);
	}


	void FrameCubemapComponentInstance::frame()
	{
		// Compute camera distance using bounds -> Use a bounding sphere to capture every axis, regardless of orientation
		const auto& bounds = getBounds();
		float cam_distance = utility::computeCameraDistance(utility::computeBoundingSphere(bounds),
			mCameraComponent->getFieldOfView());

		// Setup camera orbit controller
		glm::vec3 camera_pos = { 0.0f, 0.0f, bounds.getDepth() / 2.0f + cam_distance };
		mMeshOrbit->enable(camera_pos, {0.0f, 0.0f, 0.0f});
		mMeshOrbit->setMovementSpeed(bounds.getDiagonal() * mSpeedReference);

		// Compute camera clip planes
		// TODO: Parent skybox to camera to reduce far clip size
		float sky_scale = math::max<float>(1000.0f, bounds.getDiagonal() * 1000.0f);
		auto props = mCameraComponent->getProperties();
		props.mNearClippingPlane = math::max<float>(0.001f, cam_distance * 0.1f);
		props.mFarClippingPlane = sky_scale;
		mCameraComponent->setProperties(props);

		// Scale and center everything
		auto mesh_center = bounds.getCenter();
		mSkyTransform->setTranslate({ 0.0f, 0.0f, 0.0f });
		mSkyTransform->setUniformScale(sky_scale);
		mMeshTransform->setTranslate(-mesh_center);
		mMeshRotate->reset();
		mMeshRotate->setSpeed(0.0f);
		mSkyRenderer->setOpacity(1.0f);
	}


	void FrameCubemapComponentInstance::clear()
	{
		bind(*mTextureFallback);
	}


	const nap::TextureCube& FrameCubemapComponentInstance::getTexture() const
	{
		return mSkyRenderer->getTexture();
	}


	void FrameCubemapComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mMeshRenderer->setMesh(mMeshes[mMeshIndex]);
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
		renderService.renderObjects(window, *mCameraComponent,  { mSkyRenderer.get() });
		renderService.renderObjects(window, *mCameraComponent,  { mMeshRenderer.get() });
	}
}
