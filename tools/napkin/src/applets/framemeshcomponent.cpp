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
	RTTI_PROPERTY("BBoxTransform",		&napkin::FrameMeshComponent::mBBoxTransform,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BBoxRenderer",		&napkin::FrameMeshComponent::mBBoxRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BBoxTextRenderer",	&napkin::FrameMeshComponent::mBBoxTextRenderer, nap::rtti::EPropertyMetaData::Required)
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

		// Fetch mesh uniforms
		auto* ubo = mFlatRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mMeshColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mMeshColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		mMeshAlphaUniform = ubo->getOrCreateUniform<UniformFloatInstance>(uniform::constant::alpha);
		if (!errorState.check(mMeshAlphaUniform != nullptr, "Missing '%s' uniform", uniform::constant::alpha))
			return false;

		ubo = mBBoxRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		// Fetch bbox uniforms
		mBBoxColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mBBoxColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		// bbox min, max, center
		mBBoxTextRenderer->resize(3);

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

		// Cache some stuff
		mBounds = utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance());
		mPolyMode = mesh->getMeshInstance().getPolygonMode();
		mTopology = mesh->getMeshInstance().getDrawMode();

		// Set bbox min, max text
		if (!mBBoxTextRenderer->setText(0,
			utility::stringFormat("%.02f, %.02f, %.02f", mBounds.getMin().x, mBounds.getMin().y, mBounds.getMin().z), error))
			return false;

		if (!mBBoxTextRenderer->setText(1,
			utility::stringFormat("%.02f, %.02f, %.02f", mBounds.getMax().x, mBounds.getMax().y, mBounds.getMax().z), error))
			return
			false;

		if (!mBBoxTextRenderer->setText(2,
			utility::stringFormat("%.02f, %.02f, %.02f", mBounds.getCenter().x, mBounds.getCenter().y, mBounds.getCenter().z), error))
			return
			false;

		// Set bbox transform
		mBBoxTransform->setTranslate(mBounds.getCenter());
		mBBoxTransform->setScale(mBounds.getDimensions());

		// Set mesh for drawing
		mFlatRenderer->setMesh(render_mesh);
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
		float clip_scale = math::max<float>(1000.0f, mBounds.getDiagonal() * 1000.0f);
		auto props = mCamera->getProperties();
		props.mNearClippingPlane = math::max<float>(0.001f, cam_distance * 0.1f);
		props.mFarClippingPlane = clip_scale;
		mCamera->setProperties(props);
	}


	void FrameMeshComponentInstance::setLineWidth(float width)
	{
		mFlatRenderer->setLineWidth(width);
		mBBoxRenderer->setLineWidth(width);
	}


	void FrameMeshComponentInstance::drawMesh()
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setPolygonMode(mPolyMode);
		mFlatRenderer->getMaterialInstance().setBlendMode(mBlendMode);
		draw(mMeshColor);
	}


	void FrameMeshComponentInstance::drawWireframe()
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setPolygonMode(EPolygonMode::Line);
		mFlatRenderer->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
		draw(mWireColor);
	}


	bool FrameMeshComponentInstance::hasWireframe() const
	{
		return mMesh != nullptr ?
			utility::isTriangleMesh(mMesh->getMeshInstance()) && mPolyMode == EPolygonMode::Fill:
			false;
	}


	void FrameMeshComponentInstance::drawBounds()
	{
		// Draw bounds mesh
		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr && mMesh != nullptr);
		mBBoxColorUniform->setValue(mBBoxColor);
		std::vector<RenderableComponentInstance*> render_comps = { mBBoxRenderer.get() };
		mRenderService->renderObjects(*window, *mCamera, render_comps);

		// Draw min bbox coordinates
		auto min_screen = mCamera->worldToScreen(getBounds().getMin(), window->getRect());
		min_screen += 5.0f * window->getDisplayScale();
		mBBoxTextRenderer->setLineIndex(0);
		mBBoxTextRenderer->setLocation(min_screen);
		mBBoxTextRenderer->setColor(mBBoxColor);
		mBBoxTextRenderer->draw(*window);

		// Draw max bbox coordinates
		auto max_screen = mCamera->worldToScreen(getBounds().getMax(), window->getRect());
		max_screen += 5.0f * window->getDisplayScale();
		mBBoxTextRenderer->setLineIndex(1);
		mBBoxTextRenderer->setLocation(max_screen);
		mBBoxTextRenderer->draw(*window);

		// Draw center bbox coordinates
		auto cen_screen = mCamera->worldToScreen(getBounds().getCenter(), window->getRect());
		mBBoxTextRenderer->setLineIndex(2);
		mBBoxTextRenderer->setLocation(cen_screen);
		mBBoxTextRenderer->draw(*window);
	}


	void FrameMeshComponentInstance::draw(const RGBAColorFloat& color)
	{
		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr);

		mMeshColorUniform->setValue(color.toVec4());
		mMeshAlphaUniform->setValue(color.getAlpha());
		std::vector<RenderableComponentInstance*> render_comps = { mFlatRenderer.get()	};
		mRenderService->renderObjects(*window, *mCamera, render_comps);
	}
}

