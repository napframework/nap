/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewloadcomponent.h"

// External Includes
#include <entity.h>
#include <renderglobals.h>
#include <meshutils.h>
#include <constantshader.h>
#include <nap/core.h>
#include <renderservice.h>
#include <blinnphongcolorshader.h>

RTTI_BEGIN_CLASS(napkin::MeshPreviewLoadComponent)
	RTTI_PROPERTY("OrbitController",	&napkin::MeshPreviewLoadComponent::mOrbitController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Camera",				&napkin::MeshPreviewLoadComponent::mCamera,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlatRenderer",		&napkin::MeshPreviewLoadComponent::mFlatRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BBoxTransform",		&napkin::MeshPreviewLoadComponent::mBBoxTransform,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BBoxRenderer",		&napkin::MeshPreviewLoadComponent::mBBoxRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BBoxTextRenderer",	&napkin::MeshPreviewLoadComponent::mBBoxTextRenderer, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShadedRenderer",		&napkin::MeshPreviewLoadComponent::mShadedRenderer,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ObjectTransform",	&napkin::MeshPreviewLoadComponent::mObjectTransform,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Rotator",			&napkin::MeshPreviewLoadComponent::mRotator,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WireRenderer",		&napkin::MeshPreviewLoadComponent::mWireRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("WorldTransform",		&napkin::MeshPreviewLoadComponent::mWorldTransform,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::MeshPreviewLoadComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{

	MeshPreviewLoadComponentInstance::MeshPreviewLoadComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource) { }


	bool MeshPreviewLoadComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mRenderService != nullptr);

		// Fetch render advanced service
		mRenderAdvancedService = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(mRenderAdvancedService != nullptr);

		// Fetch normalized rotation speed
		mSpeedReference = mOrbitController->getMovementSpeed();

		// Fetch constant (flat) uniforms
		auto* ubo = mFlatRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mFlatColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mFlatColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		mFlatAlphaUniform = ubo->getOrCreateUniform<UniformFloatInstance>(uniform::constant::alpha);
		if (!errorState.check(mFlatAlphaUniform != nullptr, "Missing '%s' uniform", uniform::constant::alpha))
			return false;

		// Fetch shaded uniforms
		ubo = mShadedRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mShadedDiffuseUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::blinnphongcolor::diffuse);
		if (!errorState.check(mShadedDiffuseUniform != nullptr, "Missing '%s' uniform", uniform::blinnphongcolor::diffuse))
			return false;

		mShadedAlphaUniform = ubo->getOrCreateUniform<UniformFloatInstance>(uniform::blinnphongcolor::alpha);
		if (!errorState.check(mShadedAlphaUniform != nullptr, "Missing '%s' uniform", uniform::blinnphongcolor::alpha))
			return false;

		// Fetch bbox uniforms
		ubo = mBBoxRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mBBoxColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mBBoxColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		// Fetch wire uniforms
		ubo = mWireRenderer->getMaterialInstance().getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(ubo != nullptr, "Missing '%s' struct uniform", uniform::constant::uboStruct))
			return false;

		mWireColorUniform = ubo->getOrCreateUniform<UniformVec3Instance>(uniform::constant::color);
		if (!errorState.check(mWireColorUniform != nullptr, "Missing '%s' uniform", uniform::constant::color))
			return false;

		mWireAlphaUniform = ubo->getOrCreateUniform<UniformFloatInstance>(uniform::constant::alpha);
		if (!errorState.check(mWireAlphaUniform != nullptr, "Missing '%s' uniform", uniform::constant::alpha))
			return false;

		mWireDisplacementUniform = ubo->getOrCreateUniform<UniformFloatInstance>("displacement");
		if (!errorState.check(mWireDisplacementUniform != nullptr, "Missing 'displacement' uniform"))
			return false;

		// bbox min, max, center
		mBBoxTextRenderer->resize(3);

		// set bounding box blend properties
		mBBoxRenderer->getMaterialInstance().setBlendMode(nap::EBlendMode::Opaque);
		mBBoxRenderer->getMaterialInstance().setDepthMode(nap::EDepthMode::InheritFromBlendMode);

		// Push wire width
		setWireWidth(1.0f);

		return true;
	}


	bool MeshPreviewLoadComponentInstance::load(std::unique_ptr<IMesh>&& mesh, utility::ErrorState& error)
	{
		// Create flat mesh -> must be valid
		auto flat_mesh = mFlatRenderer->createRenderableMesh(*mesh, error);
		if (!flat_mesh.isValid())
			return false;

		// Update bounding box text -> must succeed
		auto obj_bounds = utility::computeBoundingBox<glm::vec3>(mesh->getMeshInstance());

		if (!mBBoxTextRenderer->setText(0,
			utility::stringFormat("%.02f, %.02f, %.02f", obj_bounds.getMin().x, obj_bounds.getMin().y, obj_bounds.getMin().z), error))
			return false;

		if (!mBBoxTextRenderer->setText(1,
			utility::stringFormat("%.02f, %.02f, %.02f", obj_bounds.getMax().x, obj_bounds.getMax().y, obj_bounds.getMax().z), error))
			return false;

		if (!mBBoxTextRenderer->setText(2,
			utility::stringFormat("%.02f, %.02f, %.02f", obj_bounds.getCenter().x, obj_bounds.getCenter().y, obj_bounds.getCenter().z), error))
			return false;

		// Attempt to create shaded mesh to render -> allowed to fail
		mShadedRenderMesh = RenderableMesh(); mWireRenderMesh = RenderableMesh();
		bool triangle_mesh = utility::isTriangleMesh(mesh->getMeshInstance());
		if(triangle_mesh)
		{
			// Render shaded (with lights)
			utility::ErrorState render_error;
			mShadedRenderMesh = mShadedRenderer->createRenderableMesh(*mesh, render_error);
			if (mShadedRenderMesh.isValid())
				mShadedRenderer->setMesh(mShadedRenderMesh);

			// Special wire-frame renderer -> offsets vertices for better blending
			mWireRenderMesh = mWireRenderer->createRenderableMesh(*mesh, render_error);
			if (mWireRenderMesh.isValid())
				mWireRenderer->setMesh(mWireRenderMesh);
		}

		// Cache some properties
		mPolyMode = mesh->getMeshInstance().getPolygonMode();
		mTopology = mesh->getMeshInstance().getDrawMode();
		mBlendMode = triangle_mesh ? mBlendMode : EBlendMode::Opaque;

		// Move mesh to center
		auto center_offset = -obj_bounds.getCenter();
		mObjectTransform->setTranslate(center_offset);

		// Compute bounding box xform
		mBBoxTransform->setTranslate({0.0f, 0.0f, 0.0f});
		mBBoxTransform->setScale(obj_bounds.getDimensions());

		// Compute world (global) bounds
		mWorldBounds = math::Box(
			obj_bounds.getMin() + center_offset,
			obj_bounds.getMax() + center_offset
		);

		// Change wire offset
		mWireDisplacementUniform->setValue(obj_bounds.getDiagonal() / 10000.0f);

		// Take ownership of mesh
		mMesh = std::move(mesh);
		mFlatRenderer->setMesh(flat_mesh);
		mObjectBounds = obj_bounds;

		return true;
	}


	void MeshPreviewLoadComponentInstance::frame()
	{
		// Don't frame if there's no object
		if (mMesh == nullptr)
			return;

		// Get radius of object
		float obj_radius = utility::computeBoundingSphere(mObjectBounds);

		// Compute camera distance using object bounding radius
		float cam_distance = utility::computeCameraDistance(obj_radius, mCamera->getFieldOfView());

		// Setup camera orbit controller
		glm::vec3 camera_vec = {
			0.0f,
			0.0f,
			mWorldBounds.getDepth() / 2.0f + cam_distance,
		};

		// Rotate camera vector
		auto cam_rotate = glm::rotate(glm::identity<glm::mat4>(), glm::radians(-25.0f), math::X_AXIS);
		camera_vec = cam_rotate * glm::vec4(camera_vec, 1.0f);

		// Position camera and point
		auto world_center = mWorldBounds.getCenter();
		glm::vec3 camera_pos = world_center + camera_vec;
		mOrbitController->enable(camera_pos, world_center);
		mOrbitController->setMovementSpeed(mObjectBounds.getDiagonal() * mSpeedReference);

		// Compute camera clip planes
		auto props = mCamera->getProperties();
		props.mNearClippingPlane = math::max<float>(0.001f, cam_distance * 0.075f);
		props.mFarClippingPlane  = math::max<float>(1000.0f, mObjectBounds.getDiagonal() * 100.0f);;
		mCamera->setProperties(props);

		// Stop rotation
		mRotator->reset();
		mRotator->setSpeed(0.0f);

		// Reset transform
		mWorldTransform->setScale({ 1.0f, 1.0f, 1.0f });
		mWorldTransform->setRotate(glm::identity<glm::quat>());
		mRotate = { 0.0f, 0.0f, 0.0f };
	}


	void MeshPreviewLoadComponentInstance::setWireWidth(float width)
	{
		mFlatRenderer->setLineWidth(width);
		mWireRenderer->setLineWidth(width);
		mWireWidth = width;
	}


	void MeshPreviewLoadComponentInstance::setRotate(const glm::vec3& eulerAngles)
	{
		mWorldTransform->setRotate(math::eulerToQuat(math::radians(eulerAngles)));
		mRotate = eulerAngles;
	}


	bool MeshPreviewLoadComponentInstance::isTriangleMesh() const
	{
		return mMesh != nullptr && utility::isTriangleMesh(mMesh->getMeshInstance());
	}


	int MeshPreviewLoadComponentInstance::getTriangleCount() const
	{
		return isTriangleMesh() ?
			utility::getTriangleCount(mMesh->getMeshInstance()) : 0;
	}


	void MeshPreviewLoadComponentInstance::draw()
	{
		assert(mMesh != nullptr);
		drawMesh();

		if (hasWireframe() && mDrawWireframe)
			drawWireframe();

		if (mDrawBounds)
			drawBounds();
	}


	void MeshPreviewLoadComponentInstance::drawMesh()
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setPolygonMode(mPolyMode);

		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr);

		if (mShadedRenderMesh.isValid())
		{
			mShadedDiffuseUniform->setValue(mMeshColor.toVec4());
			mShadedAlphaUniform->setValue(mMeshColor.getAlpha());
			mShadedRenderer->getMaterialInstance().setBlendMode(mBlendMode);
			mShadedRenderer->getMaterialInstance().setDepthMode(EDepthMode::InheritFromBlendMode);
			std::vector<RenderableComponentInstance*> render_comp = { mShadedRenderer.get() };
			mRenderAdvancedService->pushLights(render_comp);
			mRenderService->renderObjects(*window, *mCamera, render_comp);
			return;
		}

		mFlatColorUniform->setValue(mMeshColor.toVec4());
		mFlatAlphaUniform->setValue(mMeshColor.getAlpha());
		mFlatRenderer->getMaterialInstance().setBlendMode(mBlendMode);
		mFlatRenderer->getMaterialInstance().setDepthMode(EDepthMode::InheritFromBlendMode);
		std::vector<RenderableComponentInstance*> render_comp = { mFlatRenderer.get() };
		mRenderService->renderObjects(*window, *mCamera, render_comp);
	}


	void MeshPreviewLoadComponentInstance::drawWireframe()
	{
		assert(mMesh != nullptr);
		mMesh->getMeshInstance().setPolygonMode(EPolygonMode::Line);

		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr);

		if (mWireRenderMesh.isValid())
		{
			mWireColorUniform->setValue(mWireColor.toVec4());
			mWireAlphaUniform->setValue(mWireColor.getAlpha());
			mWireRenderer->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
			mFlatRenderer->getMaterialInstance().setDepthMode(EDepthMode::InheritFromBlendMode);
			std::vector<RenderableComponentInstance*> render_comp = { mWireRenderer.get() };
			mRenderService->renderObjects(*window, *mCamera, render_comp);
			return;
		}

		mFlatColorUniform->setValue(mWireColor.toVec4());
		mFlatAlphaUniform->setValue(mWireColor.getAlpha());
		mFlatRenderer->getMaterialInstance().setBlendMode(EBlendMode::AlphaBlend);
		mFlatRenderer->getMaterialInstance().setDepthMode(EDepthMode::InheritFromBlendMode);
		std::vector<RenderableComponentInstance*> render_comp = { mFlatRenderer.get() };
		mRenderService->renderObjects(*window, *mCamera, render_comp);
	}


	bool MeshPreviewLoadComponentInstance::hasWireframe() const
	{
		return mMesh != nullptr ?
			utility::isTriangleMesh(mMesh->getMeshInstance()) && mPolyMode == EPolygonMode::Fill:
			false;
	}


	void MeshPreviewLoadComponentInstance::drawBounds()
	{
		// Draw bounds mesh
		auto* window = mRenderService->getCurrentRenderWindow();
		assert(window != nullptr && mMesh != nullptr);
		mBBoxColorUniform->setValue(mBBoxColor);
		std::vector<RenderableComponentInstance*> render_comps = { mBBoxRenderer.get() };
		mRenderService->renderObjects(*window, *mCamera, render_comps);

		// Draw min bbox coordinates
		auto min_xform = mObjectTransform->getGlobalTransform() * glm::vec4(mObjectBounds.getMin(), 1.0f);
		auto min_screen = mCamera->worldToScreen(min_xform, window->getRect());
		min_screen += 5.0f * window->getDisplayScale();
		mBBoxTextRenderer->setLineIndex(0);
		mBBoxTextRenderer->setLocation(min_screen);
		mBBoxTextRenderer->setColor(mBBoxColor);
		mBBoxTextRenderer->draw(*window);

		// Draw max bbox coordinates
		auto max_xform = mObjectTransform->getGlobalTransform() * glm::vec4(mObjectBounds.getMax(), 1.0f);
		auto max_screen = mCamera->worldToScreen(max_xform, window->getRect());
		max_screen += 5.0f * window->getDisplayScale();
		mBBoxTextRenderer->setLineIndex(1);
		mBBoxTextRenderer->setLocation(max_screen);
		mBBoxTextRenderer->draw(*window);

		// Draw center bbox coordinates
		auto cen_xform = mObjectTransform->getGlobalTransform() * glm::vec4(mObjectBounds.getCenter(), 1.0f);
		auto cen_screen = mCamera->worldToScreen(cen_xform, window->getRect());
		mBBoxTextRenderer->setLineIndex(2);
		mBBoxTextRenderer->setLocation(cen_screen);
		mBBoxTextRenderer->draw(*window);
	}


	void MeshPreviewLoadComponentInstance::clear()
	{
		mWireRenderMesh = RenderableMesh();
		mShadedRenderMesh = RenderableMesh();
		mMesh.reset(nullptr);
	}
}

