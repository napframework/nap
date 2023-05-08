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
	RTTI_PROPERTY("MovementSpeed",			&nap::OrthoController::mMovementSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ZoomSpeed",				&nap::OrthoController::mZoomSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OrthoCameraComponent",	&nap::OrthoController::mOrthoCameraComponent,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable",					&nap::OrthoController::mEnable,					nap::rtti::EPropertyMetaData::Default)
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
		mEnabled = getComponent<OrthoController>()->mEnable;

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
		auto props = mOrthoCameraComponent->getProperties();
		props.mLeftPlane = glm::sign(props.mLeftPlane) * mCameraScale;
		props.mRightPlane = glm::sign(props.mRightPlane) * mCameraScale;
		props.mTopPlane = glm::sign(props.mTopPlane) * mCameraScale;
		props.mBottomPlane = glm::sign(props.mBottomPlane) * mCameraScale;

		mOrthoCameraComponent->setProperties(props);
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
			const glm::ivec2& rt_size = mOrthoCameraComponent->getRenderTargetSize();
			mMousePosAtClick = glm::vec2(pointerPressEvent.mX / static_cast<float>(rt_size.x), pointerPressEvent.mY / static_cast<float>(rt_size.y));

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
		const auto& resource = getComponent<OrthoController>();
		const glm::ivec2& rt_size = mOrthoCameraComponent->getRenderTargetSize();
		float aspect_ratio = static_cast<float>(rt_size.y) / static_cast<float>(rt_size.x);

		if (mMode == EMode::Pan)
		{
			// Calculate aspect correct scale based on uniform mCameraScale
			glm::vec2 aspect_correct_scale = { mCameraScale / static_cast<float>(rt_size.x), (mCameraScale * aspect_ratio) / static_cast<float>(rt_size.y) };

			// Translate on current world 'right' and 'up' axes
			glm::vec3 right_translate = static_cast<float>(-pointerMoveEvent.mRelX) * aspect_correct_scale.x * mTransformComponent->getGlobalTransform()[0];
			glm::vec3 up_translate = static_cast<float>(pointerMoveEvent.mRelY) * aspect_correct_scale.y * mTransformComponent->getGlobalTransform()[1];
			glm::vec3 delta = (right_translate - up_translate) * resource->mMovementSpeed;

			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + delta);
		}
		else if (mMode == EMode::Zoom)
		{
			// Zooming works on both axes
			int pointer_move = -pointerMoveEvent.mRelX;
			if (std::abs(pointerMoveEvent.mRelY) > std::abs(pointerMoveEvent.mRelX))
				pointer_move = pointerMoveEvent.mRelY;

			// Animate scale based on input and zoomspeed
			mCameraScale += static_cast<float>(pointer_move) * resource->mZoomSpeed;
			mCameraScale = glm::max(1.0f/1000.0f, mCameraScale);

			// We want to zoom on the mouse cursor position that was set when the mouse button was pressed.
			// This is done by calculating the difference in translation for that specific position, using both the
			// scale when the mouse was pressed and the new scale. By subtracting that difference in translation we
			// achieve 'zoom around cursor'

			// Calculate the aspect correct scale for the 'scale on click'.
			glm::vec2 aspect_correct_scale = { mCameraScale, mCameraScale * aspect_ratio };
			glm::vec2 aspect_correct_scale_at_click = { mCameraScaleAtClick, mCameraScaleAtClick * aspect_ratio };

			// Calculate difference in translation on mouse click and now
			glm::vec2 translate_before_zoom = mMousePosAtClick * aspect_correct_scale_at_click;
			glm::vec2 translate_after_zoom = mMousePosAtClick * aspect_correct_scale;
			glm::vec2 diff = translate_before_zoom - translate_after_zoom;

			// Correct delta translation to achieve zoom around cursor
			glm::vec3 right_translate = diff.x * mTransformComponent->getGlobalTransform()[0];
			glm::vec3 up_translate = diff.y * mTransformComponent->getGlobalTransform()[1];
			mTransformComponent->setTranslate(mTranslateAtClick + right_translate + up_translate);

			updateCameraProperties();
		}
	}
}
