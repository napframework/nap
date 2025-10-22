/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Internal includes
#include "texturepreviewload2dcomponent.h"

// External includes
#include <entity.h>
#include <textureshader.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <renderglobals.h>
#include <meshutils.h>
#include <spheremesh.h>

// nap::appletcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::TexturePreviewLoad2DComponent)
	RTTI_PROPERTY("ZoomPanController",		&napkin::TexturePreviewLoad2DComponent::mZoomPanController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneTransform",			&napkin::TexturePreviewLoad2DComponent::mPlaneTransform,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneRenderer",			&napkin::TexturePreviewLoad2DComponent::mPlaneRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneCamera",			&napkin::TexturePreviewLoad2DComponent::mPlaneCamera,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRenderer",			&napkin::TexturePreviewLoad2DComponent::mMeshRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshCamera",				&napkin::TexturePreviewLoad2DComponent::mMeshCamera,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshRotate",				&napkin::TexturePreviewLoad2DComponent::mMeshRotate,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshTransform",			&napkin::TexturePreviewLoad2DComponent::mMeshTransform,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MeshOrbit",				&napkin::TexturePreviewLoad2DComponent::mMeshOrbit,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FallbackTexture",		&napkin::TexturePreviewLoad2DComponent::mFallbackTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Meshes",					&napkin::TexturePreviewLoad2DComponent::mMeshes,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::appletcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::TexturePreviewLoad2DComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	bool TexturePreviewLoad2DComponentInstance::init(utility::ErrorState& errorState)
	{
		//////////////////////////////////////////////////////////////////////////
		// Orthographic Zoom & Pan Plane
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
		// Perspective Mesh
		//////////////////////////////////////////////////////////////////////////

		// Create a link between the material and mesh for every mesh you can select from
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		TexturePreviewLoad2DComponent* resource = getComponent<TexturePreviewLoad2DComponent>();
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

		//////////////////////////////////////////////////////////////////////////
		// Rest...
		//////////////////////////////////////////////////////////////////////////

		// Texture to use when selection is cleared
		mTextureFallback = getComponent<TexturePreviewLoad2DComponent>()->mFallbackTexture.get();

		// Reference orbit move speed
		mSpeedReference = mMeshOrbit->getMovementSpeed();

		// Plane bounds
		mPlaneBounds = utility::computeBoundingBox<glm::vec3>(mPlaneRenderer->getMesh().getMeshInstance());

		// Bind fall-back texture
		bind(*mTextureFallback);

		return true;
	}


	void TexturePreviewLoad2DComponentInstance::load(std::unique_ptr<Texture2D> texure)
	{
		bind(*texure);
		mTexture = std::move(texure);
	}


	int TexturePreviewLoad2DComponentInstance::load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error)
	{
		// Catch most obvious explicit error -> missing uv attribute
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::uv) != nullptr,
			"Unable to bind texture, '%s' has no % s vertex attribute", mesh->mID.c_str(), vertexid::uv))
			return -1;

		// Try and create a render-able mesh
		RenderableMesh render_mesh = mMeshRenderer->createRenderableMesh(*mesh, error);
		if (!render_mesh.isValid())
			return -1;

		// Replace custom mesh
		if (hasMeshLoaded())
		{
			mMeshes.pop_back();
			mBounds.pop_back();
		}

		// Add renderable mesh, bbox and cache
		mMeshes.emplace_back(render_mesh);
		mBounds.emplace_back(utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance()));
		mMesh = std::move(mesh);

		// Bind for rendering if selected
		setMeshIndex(getMeshIndex());
		return mMeshes.size() - 1;
	}


	void TexturePreviewLoad2DComponentInstance::bind(Texture2D& texture)
	{
		// Bind texture to plane and mesh shader
		assert(mPlaneSampler != nullptr);
		mPlaneSampler->setTexture(texture); 
		assert(mMeshSampler != nullptr);
		mMeshSampler->setTexture(texture);
	}


	void TexturePreviewLoad2DComponentInstance::frame()
	{
		// 2D Projection
		assert(mPlaneSampler->hasTexture());
		mZoomPanController->frameTexture(mPlaneSampler->getTexture(), *mPlaneTransform);
		mPlaneOpacity->setValue(1.0f);

		// Get object radius -> bounding-sphere or radius of sphere
		auto bounds = getBounds();
		float obj_radius = getMesh().get_type().is_derived_from(RTTI_OF(nap::SphereMesh)) ?
			static_cast<const SphereMesh&>(getMesh()).mRadius : utility::computeBoundingSphere(bounds);

		// Compute mesh camera distance using bounds
		float cam_distance = utility::computeCameraDistance(obj_radius, mMeshCamera->getFieldOfView());

		// Setup camera orbit controller
		glm::vec3 camera_pos = { 0.0f, 0.0f, (bounds.getDepth() / 2.0f) + cam_distance };
		mMeshOrbit->enable(camera_pos, {0.0f, 0.0f, 0.0f});
		mMeshOrbit->setMovementSpeed(mSpeedReference * bounds.getDiagonal());

		// Compute mesh camera clip planes and set
		auto props = mMeshCamera->getProperties();
		props.mNearClippingPlane = math::max<float>(0.001f, cam_distance * 0.1f);
		props.mFarClippingPlane = math::max<float>(1.0f, bounds.getDiagonal() * 100.0f);
		mMeshCamera->setProperties(props);

		// Scale and center everything -> move mesh to center
		auto mesh_center = bounds.getCenter();
		mMeshTransform->setTranslate(-bounds.getCenter());
		mMeshRotate->reset();
		mMeshRotate->setSpeed(0.0f);
	}


	const Texture2D& TexturePreviewLoad2DComponentInstance::getTexture() const
	{
		assert(mPlaneSampler != nullptr && mPlaneSampler->hasTexture());
		return mPlaneSampler->getTexture();
	}


	void TexturePreviewLoad2DComponentInstance::setOpacity(float opacity)
	{
		switch (mMode)
		{
		case EMode::Plane:
			assert(mPlaneOpacity != nullptr);
			mPlaneOpacity->setValue(opacity);
			break;
		default:
			assert(false);
			break;
		}
	}


	float TexturePreviewLoad2DComponentInstance::getOpacity() const
	{
		switch (mMode)
		{
		case EMode::Plane:
			assert(mPlaneOpacity != nullptr);
			return mPlaneOpacity->getValue();
			break;
		default:
			assert(false);
			break;
		}
		return 0.0f;
	}


	void TexturePreviewLoad2DComponentInstance::setMeshIndex(int index)
	{
		mMeshIndex = math::clamp<int>(index, 0, mMeshes.size() - 1);
		mMeshRenderer->setMesh(mMeshes[mMeshIndex]);
	}


	const nap::math::Box& TexturePreviewLoad2DComponentInstance::getBounds() const
	{
		return mMode == EMode::Plane ? mPlaneBounds :
			mBounds[mMeshIndex];
	}


	const IMesh& TexturePreviewLoad2DComponentInstance::getMesh()
	{
		assert(mMeshes.size() > mMeshIndex);
		return mMode == EMode::Plane ? mPlaneRenderer->getMesh() :
			mMeshes[mMeshIndex].getMesh();
	}


	void TexturePreviewLoad2DComponentInstance::processWindowEvents(InputService& inputService, RenderWindow& window)
	{
		static nap::DefaultInputRouter input_router;
		switch (mMode)
		{
		case napkin::TexturePreviewLoad2DComponentInstance::EMode::Plane:
			inputService.processWindowEvents(window, input_router, { mPlaneCamera->getEntityInstance() });
			break;
		case napkin::TexturePreviewLoad2DComponentInstance::EMode::Mesh:
			inputService.processWindowEvents(window, input_router, { mMeshCamera->getEntityInstance() });
			break;
		default:
			assert(false);
			break;
		}
	}


	void TexturePreviewLoad2DComponentInstance::draw(RenderService& renderService, RenderWindow& window)
	{
		switch (mMode)
		{
		case napkin::TexturePreviewLoad2DComponentInstance::EMode::Plane:
			renderService.renderObjects(window, *mPlaneCamera, { mPlaneRenderer.get() });
			break;
		case napkin::TexturePreviewLoad2DComponentInstance::EMode::Mesh:
			renderService.renderObjects(window, *mMeshCamera, { mMeshRenderer.get() });
			break;
		default:
			assert(false);
			break;
		}
	}
}
