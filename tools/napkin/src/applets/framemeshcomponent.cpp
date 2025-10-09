/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "framemeshcomponent.h"

// External Includes
#include <entity.h>
#include <renderglobals.h>
#include <meshutils.h>
#include <constantshader.h>
#include <nap/core.h>
#include <renderservice.h>

// nap::framemeshcomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::FrameMeshComponent)
	RTTI_PROPERTY("OrbitController",	&napkin::FrameMeshComponent::mOrbitController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Camera",				&napkin::FrameMeshComponent::mCamera,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlatRenderer",		&napkin::FrameMeshComponent::mFlatRenderer,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::framemeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::FrameMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{

	FrameMeshComponentInstance::FrameMeshComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource) { }


	bool FrameMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mRenderService != nullptr);

		// Fetch normalized rotation speed
		mSpeedReference = mOrbitController->getMovementSpeed();

		// Fetch uniforms
		auto* ubo = mFlatRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		mAlphaUniform = ubo->getOrCreateUniform<UniformFloatInstance>(uniform::constant::alpha);
		if (!errorState.check(mAlphaUniform != nullptr, "Missing '%s' uniform", uniform::constant::alpha))
			return false;

		return true;
	}


	bool FrameMeshComponentInstance::load(std::unique_ptr<IMesh>&& mesh, utility::ErrorState& error)
	{
		// Attempt to create mesh to render
		auto render_mesh = mFlatRenderer->createRenderableMesh(*mesh, error);
		if (!render_mesh.isValid())
			return false;

		// Find position attr
		if (!error.check(mesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::position) != nullptr,
			"%s: missing '%s' (vec3) vertex attribute", mesh->mID.c_str(), vertexid::position))
			return false;

		// Update the mesh
		mBounds = utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance());
		mFlatRenderer->setMesh(render_mesh);

		// Copy ownership
		mMesh = std::move(mesh);
		return true;
	}


	void FrameMeshComponentInstance::frame()
	{
		// Don't frame if there's no object
		if (mMesh == nullptr)
			return;

		// Get radius of object
		float obj_radius = utility::computeBoundingSphere(mBounds);

		// Compute camera distance using object bounding radius
		float cam_distance = utility::computeCameraDistance(obj_radius, mCamera->getFieldOfView());

		// Setup camera orbit controller
		auto center = mBounds.getCenter();
		glm::vec3 camera_pos = { center.x, center.y, mBounds.getDepth() / 2.0f + cam_distance };
		mOrbitController->enable(camera_pos, center);
		mOrbitController->setMovementSpeed(mBounds.getDiagonal() * mSpeedReference);

		// Compute camera clip planes
		// TODO: Parent skybox to camera to reduce far clip size
		float sky_scale = math::max<float>(1000.0f, mBounds.getDiagonal() * 1000.0f);
		auto props = mCamera->getProperties();
		props.mNearClippingPlane = math::max<float>(0.001f, cam_distance * 0.1f);
		props.mFarClippingPlane = sky_scale;
		mCamera->setProperties(props);
	}


	void FrameMeshComponentInstance::drawMesh(const RGBAColorFloat& color)
	{
		mFlatRenderer->getMaterialInstance().setBlendMode(EBlendMode::Additive);
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setDrawMode(EDrawMode::Triangles);
		draw(color);
	}


	void FrameMeshComponentInstance::drawWireframe(const RGBAColorFloat& color, float width)
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setDrawMode(EDrawMode::Lines);
		mFlatRenderer->setLineWidth(width);
		mFlatRenderer->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
		draw(color);
	}


	void FrameMeshComponentInstance::drawPoints(const RGBAColorFloat& color)
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setDrawMode(EDrawMode::Points);
		draw(color);
	}


	void FrameMeshComponentInstance::draw(const RGBAColorFloat& color)
	{
		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr);

		mColorUniform->setValue(color.toVec4());
		mAlphaUniform->setValue(color.getAlpha());
		std::vector<RenderableComponentInstance*> render_comps = { mFlatRenderer.get()	};
		mRenderService->renderObjects(*window, *mCamera, render_comps);
	}
}

