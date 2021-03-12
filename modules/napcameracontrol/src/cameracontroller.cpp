/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cameracontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"

#include <entity.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::CameraController)
	RTTI_PROPERTY("LookAtTarget",	&nap::CameraController::mLookAtTarget,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CameraControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	CameraControllerInstance::CameraControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool CameraControllerInstance::init(utility::ErrorState& errorState)
	{
		// KeyInputComponent is required to receive input
		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "%s: missing KeyInputComponent", mID.c_str()))
			return false;

		mOrbitComponent = getEntityInstance()->findComponent<OrbitControllerInstance>();
		if (!errorState.check(mOrbitComponent != nullptr, "%s: missing OrbitControllerInstance", mID.c_str()))
			return false;

		mFirstPersonComponent = getEntityInstance()->findComponent<FirstPersonControllerInstance>();
		if (!errorState.check(mFirstPersonComponent != nullptr, "%s: missing FirstPersonControllerInstance", mID.c_str()))
			return false;

		mOrthoComponent = getEntityInstance()->findComponent<OrthoControllerInstance>();
		if (!errorState.check(mOrthoComponent != nullptr, "%s: missing OrthoControllerInstance", mID.c_str()))
			return false;

		key_component->pressed.connect(std::bind(&CameraControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&CameraControllerInstance::onKeyRelease, this, std::placeholders::_1));

		storeLastPerspTransform();
		switchMode(mMode);

		return true; 
	}


	CameraComponentInstance& CameraControllerInstance::getCameraComponent()
	{
		if (mMode == ECameraMode::FirstPerson)
			return mFirstPersonComponent->getCameraComponent();
		else if (mMode == ECameraMode::Orbit)
			return mFirstPersonComponent->getCameraComponent();
		else
			return mOrthoComponent->getCameraComponent();
	}


	/**
	 * Stores the current perspective transform. This is done to restore the perspective transform when switching
	 * back to perspective mode.
	 */
	void CameraControllerInstance::storeLastPerspTransform()
	{
		TransformComponentInstance& transform = getEntityInstance()->getComponent<nap::TransformComponentInstance>();
		mLastPerspPos = transform.getTranslate();
		mLastPerspRotate = transform.getRotate();
	}


	/**
	 * Helper to switch mode. Enables controllers, sets them in the correct state.
	 */
	void CameraControllerInstance::switchMode(ECameraMode targetMode)
	{
		if (targetMode == ECameraMode::FirstPerson)
		{
			// If we're switching back from orthographic camera, enable controller while resetting position to last know position
			if ((mMode & ECameraMode::Orthographic) != ECameraMode::None)
				mFirstPersonComponent->enable(mLastPerspPos, mLastPerspRotate);
			else
				mFirstPersonComponent->enable();

			mOrbitComponent->disable();
			mOrthoComponent->disable();
		}
		else if (targetMode == ECameraMode::Orbit)
		{
			nap::TransformComponentInstance& lookAtTransform = mLookAtTarget->getComponent<nap::TransformComponentInstance>();

			// If we're switching back from orthographic camera, enable controller while resetting position to last know position
			if ((mMode & ECameraMode::Orthographic) != ECameraMode::None)
				mOrbitComponent->enable(mLastPerspPos, lookAtTransform.getTranslate());
			else
				mOrbitComponent->enable(lookAtTransform.getTranslate());

			mFirstPersonComponent->disable();
			mOrthoComponent->disable();
		}
		else
		{
			// Remember the current perspective transform before we alter the transform
			if ((mMode & ECameraMode::Perspective) != ECameraMode::None)
				storeLastPerspTransform();

			// Depending on orthographic mode, make a rotation. 
			glm::vec3 camera_translate_axis;
			glm::quat rotation;
			switch (targetMode)
			{
				case ECameraMode::OrthographicTop:
					camera_translate_axis = glm::vec3(0.0f, -1.0f, 0.0f);
					rotation = glm::angleAxis((float)-math::PI_2, glm::vec3(1.0f, 0.0f, 0.0f));
					break;

				case ECameraMode::OrthographicBottom:
					camera_translate_axis = glm::vec3(0.0f, 1.0f, 0.0f);
					rotation = glm::angleAxis((float)math::PI_2, glm::vec3(1.0f, 0.0f, 0.0f));
					break;

				case ECameraMode::OrthographicLeft:
					camera_translate_axis = glm::vec3(1.0f, 0.0f, 0.0f);
					rotation = glm::angleAxis((float)-math::PI_2, glm::vec3(0.0f, 1.0f, 0.0f));
					break;

				case ECameraMode::OrthographicRight:
					camera_translate_axis = glm::vec3(-1.0f, 0.0f, 0.0f);
					rotation = glm::angleAxis((float)math::PI_2, glm::vec3(0.0f, 1.0f, 0.0f));
					break;

				case ECameraMode::OrthographicFront:
					camera_translate_axis = glm::vec3(0.0f, 0.0f, -1.0f);
					rotation = glm::angleAxis((float)0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
					break;

				case ECameraMode::OrthographicBack:
					camera_translate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
					rotation = glm::angleAxis((float)math::PI, glm::vec3(0.0f, 1.0f, 0.0f));
					break;
			}

			// The translation is placed some distance from the lookat target
			nap::TransformComponentInstance& lookat_transform = mLookAtTarget->getComponent<nap::TransformComponentInstance>();
			glm::vec3 target_pos(lookat_transform.getTranslate());
			const float distance = 100.0f;
			glm::vec3 camera_translate = target_pos - camera_translate_axis * distance;

			mOrthoComponent->enable(camera_translate, rotation);
			mFirstPersonComponent->disable();
			mOrbitComponent->disable();
		}
		mMode = targetMode;
	}


	void CameraControllerInstance::onKeyPress(const KeyPressEvent& keyReleaseEvent)
	{
		switch (keyReleaseEvent.mKey)
		{
		case EKeyCode::KEY_LALT:
			// Only switch to orbit mode if we're already in perspective mode
			if ((mMode & ECameraMode::Perspective) != ECameraMode::None)
				switchMode(ECameraMode::Orbit);
		}
	}


	void CameraControllerInstance::onKeyRelease(const KeyReleaseEvent& keyReleaseEvent)
	{
		switch (keyReleaseEvent.mKey)
		{
		case EKeyCode::KEY_LALT:
			if ((mMode & ECameraMode::Perspective) != ECameraMode::None)
				switchMode(ECameraMode::FirstPerson);
			break;

		case EKeyCode::KEY_p:
			switchMode(ECameraMode::FirstPerson);
			break;

		case EKeyCode::KEY_1:
			switchMode(ECameraMode::OrthographicTop);
			break;

		case EKeyCode::KEY_2:
			switchMode(ECameraMode::OrthographicBottom);
			break;

		case EKeyCode::KEY_3:
			switchMode(ECameraMode::OrthographicLeft);
			break;

		case EKeyCode::KEY_4:
			switchMode(ECameraMode::OrthographicRight);
			break;

		case EKeyCode::KEY_5:
			switchMode(ECameraMode::OrthographicFront);
			break;

		case EKeyCode::KEY_6:
			switchMode(ECameraMode::OrthographicBack);
			break;
		}
	}
}