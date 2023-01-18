/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "orthocontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <entity.h>
#include <mathutils.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

RTTI_BEGIN_CLASS(nap::OrthoController)
	RTTI_PROPERTY("ZoomSpeed",				&nap::OrthoController::mZoomSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OrthoCameraComponent",	&nap::OrthoController::mOrthoCameraComponent,	nap::rtti::EPropertyMetaData::Default)
	RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrthoControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	void OrthoController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
		components.push_back(RTTI_OF(KeyInputComponent));
	}

	//////////////////////////////////////////////////////////////////////////

	OrthoControllerInstance::OrthoControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool OrthoControllerInstance::init(utility::ErrorState& errorState)
	{
		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		pointer_component->pressed.connect(std::bind(&OrthoControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&OrthoControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&OrthoControllerInstance::onMouseUp, this, std::placeholders::_1));

		// The CorrectAspectRatio mode will correct the height based on the aspect ratio
		mOrthoCameraComponent->setMode(EOrthoCameraMode::CorrectAspectRatio);

		return true;
	}


	CameraComponentInstance& OrthoControllerInstance::getCameraComponent()
	{
		return *mOrthoCameraComponent;
	}


	void OrthoControllerInstance::updateCameraProperties()
	{
		// In this code we assume a uniform scale for the width and height. In the 'CorrectAspectRatio' mode,
		// the camera itself will correct the height based on the aspect ratio
		nap::OrthoCameraProperties camera_properties = mOrthoCameraComponent->getProperties();
		float half_scale = mCameraScale * 0.5f;
		camera_properties.mLeftPlane = -half_scale;
		camera_properties.mRightPlane = half_scale;
		camera_properties.mTopPlane = -half_scale;
		camera_properties.mBottomPlane = half_scale;
		mOrthoCameraComponent->setProperties(camera_properties);
	}


	void OrthoControllerInstance::enable(const glm::vec3& cameraPos, const glm::quat& cameraRotate)
	{
		mTransformComponent->setTranslate(cameraPos);
		mTransformComponent->setRotate(cameraRotate);

		updateCameraProperties();

		mEnabled = true;
	}


	void OrthoControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		if (pointerPressEvent.mButton == PointerClickEvent::EButton::LEFT)
		{
			mMode = EMode::Pan;
		}
		else if (pointerPressEvent.mButton == PointerClickEvent::EButton::RIGHT)
		{
			mMode = EMode::Zoom;

			// Transform mouse pos into normalized coords (-1..1)
			glm::ivec2 render_target_size = mOrthoCameraComponent->getRenderTargetSize();
			mMousePosAtClick = glm::vec2(pointerPressEvent.mX / (float)render_target_size.x, pointerPressEvent.mY / (float)render_target_size.y) * 2.0f - 1.0f;

			// Store camera translation and scale
			mTranslateAtClick = mTransformComponent->getTranslate();
			mCameraScaleAtClick = mCameraScale;
		}
	}


	void OrthoControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		mMode = EMode::None;
	}


	void OrthoControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (mMode == EMode::Pan)
		{
			// Calculate aspect correct scale based on uniform mCameraScale
			glm::ivec2 render_target_size = mOrthoCameraComponent->getRenderTargetSize();
			float aspect_ratio = (float)render_target_size.y / (float)render_target_size.x;
			glm::vec2 aspect_correct_scale = glm::vec2(mCameraScale / (float)render_target_size.x, (mCameraScale * aspect_ratio) / (float)render_target_size.y);

			// Translate on current world 'right' and 'up' axes
			const glm::mat4& camera_transform = mTransformComponent->getGlobalTransform();
			glm::vec3 right_translate = (float)-pointerMoveEvent.mRelX * aspect_correct_scale.x * camera_transform[0];
			glm::vec3 up_translate = (float)pointerMoveEvent.mRelY * aspect_correct_scale.y * camera_transform[1];
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + right_translate + up_translate);
		}
		else if (mMode == EMode::Zoom)
		{
			// Zooming works on both axes
			int pointerMove = -pointerMoveEvent.mRelX;
			if (abs(pointerMoveEvent.mRelY) > abs(pointerMoveEvent.mRelX))
				pointerMove = pointerMoveEvent.mRelY;

			// Animate scale based on input and zoomspeed
			mCameraScale += (float)pointerMove * getComponent<OrthoController>()->mZoomSpeed;
			mCameraScale = glm::max(1.0f, mCameraScale);

			// We want to zoom on the mouse cursor position that was set when the mouse button was pressed.
			// This is done by calculating the difference in translation for that specific position, using both the
			// scale when the mouse was pressed and the new scale. By subtracting that difference in translation we
			// achieve 'zoom around cursor'

			// Calculate the aspect correct scale for both the 'scale on click' and the new scale.
			glm::ivec2 render_target_size = mOrthoCameraComponent->getRenderTargetSize();
			float aspect_ratio = (float)render_target_size.y / (float)render_target_size.x;
			glm::vec2 aspect_correct_scale_at_click(mCameraScaleAtClick, mCameraScaleAtClick * aspect_ratio);
			glm::vec2 aspect_correct_scale(mCameraScale, mCameraScale * aspect_ratio);

			// Calculate difference in translation on mouse click and now
			glm::vec2 translate_before_zoom = mMousePosAtClick * aspect_correct_scale_at_click * 0.5f;
			glm::vec2 translate_after_zoom = mMousePosAtClick * aspect_correct_scale * 0.5f;
			glm::vec2 delta = translate_before_zoom - translate_after_zoom;

			// Correct delta translation to achieve zoom around cursor
			const glm::mat4& camera_transform = mTransformComponent->getGlobalTransform();
			glm::vec3 right_translate = (float)delta.x * camera_transform[0];
			glm::vec3 up_translate = (float)delta.y * camera_transform[1];
			mTransformComponent->setTranslate(mTranslateAtClick + right_translate + up_translate);

			updateCameraProperties();
		}
	}
}
