/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "texturepreviewloadcubecomponent.h"

// External Includes
#include <entity.h>
#include <renderskyboxcomponent.h>
#include <mathutils.h>
#include <inputrouter.h>
#include <renderglobals.h>
#include <meshutils.h>
#include <spheremesh.h>

// nap::framecubemapcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::TexturePreviewLoadCubeComponent)
	RTTI_PROPERTY("SkyboxRenderer",			&napkin::TexturePreviewLoadCubeComponent::mSkyRenderer,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SkboxTransform",			&napkin::TexturePreviewLoadCubeComponent::mSkyTransform,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",		&napkin::TexturePreviewLoadCubeComponent::mCameraComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshOrbit",				&napkin::TexturePreviewLoadCubeComponent::mMeshOrbit,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRenderer",			&napkin::TexturePreviewLoadCubeComponent::mMeshRenderer,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRotate",				&napkin::TexturePreviewLoadCubeComponent::mMeshRotate,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshTransform",			&napkin::TexturePreviewLoadCubeComponent::mMeshTransform,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",		&napkin::TexturePreviewLoadCubeComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Meshes",					&napkin::TexturePreviewLoadCubeComponent::mMeshes,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::framecubemapcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::TexturePreviewLoadCubeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{

	bool TexturePreviewLoadCubeComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch reflective mesh cubemap sampler input
		mReflectiveCubeSampler = mMeshRenderer->getMaterialInstance().getOrCreateSampler<SamplerCubeInstance>("environmentMap");
		if (!errorState.check(mReflectiveCubeSampler != nullptr, "Missing cube sampler input '%s'", "environmentMap"))
			return false;


		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		TexturePreviewLoadCubeComponent* resource = getComponent<TexturePreviewLoadCubeComponent>();
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
		mTextureFallback = getComponent<TexturePreviewLoadCubeComponent>()->mFallbackTexture.get();

		// Fetch normalized rotation speed
		mSpeedReference = mMeshOrbit->getMovementSpeed();

		// Setup
		setMeshIndex(0);
		bind(*mTextureFallback);

		return true;
	}


	void TexturePreviewLoadCubeComponentInstance::load(std::unique_ptr<TextureCube> texure)
	{
		// Bind new texture
		bind(*texure);

		// Replace active texture
		mTexture = std::move(texure);
	}


	int TexturePreviewLoadCubeComponentInstance::load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error)
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


	void TexturePreviewLoadCubeComponentInstance::bind(TextureCube& texture)
	{
		mSkyRenderer->setTexture(texture);
		mReflectiveCubeSampler->setTexture(texture);
	}


	void TexturePreviewLoadCubeComponentInstance::frame()
	{
		// Get object radius -> bounding-sphere or radius of sphere
		auto bounds = getBounds();
		float obj_radius = getMesh().get_type().is_derived_from(RTTI_OF(nap::SphereMesh)) ?
			static_cast<const SphereMesh&>(getMesh()).mRadius : utility::computeBoundingSphere(bounds);

		// Compute camera distance using object bounding radius
		float cam_distance = utility::computeCameraDistance(obj_radius, mCameraComponent->getFieldOfView());

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


	void TexturePreviewLoadCubeComponentInstance::clear()
	{
		bind(*mTextureFallback);
	}


	const nap::TextureCube& TexturePreviewLoadCubeComponentInstance::getTexture() const
	{
		return mSkyRenderer->getTexture();
	}


	void TexturePreviewLoadCubeComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mMeshRenderer->setMesh(mMeshes[mMeshIndex]);
	}


	const IMesh& TexturePreviewLoadCubeComponentInstance::getMesh()
	{
		assert(mMeshes.size() > mMeshIndex);
		return mMeshes[mMeshIndex].getMesh();
	}


	bool TexturePreviewLoadCubeComponentInstance::hasMeshLoaded() const
	{
		return mMesh != nullptr;
	}


	void TexturePreviewLoadCubeComponentInstance::processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window)
	{
		static DefaultInputRouter input_router;
		inputService.processWindowEvents(window, input_router, { mCameraComponent->getEntityInstance() });
	}


	void TexturePreviewLoadCubeComponentInstance::draw(RenderService& renderService, RenderWindow& window)
	{
		// First draw skybox, then reflective mesh
		renderService.renderObjects(window, *mCameraComponent,  { mSkyRenderer.get() });
		renderService.renderObjects(window, *mCameraComponent,  { mMeshRenderer.get() });
	}
}
