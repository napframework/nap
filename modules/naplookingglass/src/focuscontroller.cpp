/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "focuscontroller.h"
#include <inputevent.h>
#include <inputcomponent.h>
#include <transformcomponent.h>
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/orthonormalize.hpp>

RTTI_BEGIN_CLASS(nap::FocusController)
	RTTI_PROPERTY("ZoomSpeed",				&nap::FocusController::mZoomSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MovementSpeed",			&nap::FocusController::mMovementSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateSpeed",			&nap::FocusController::mRotateSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinZoomDistance",		&nap::FocusController::mMinZoomDistance,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AutoFocus",				&nap::FocusController::mAutoFocus,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LookAtTarget",			&nap::FocusController::mLookAtTargetComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FocusControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	FocusControllerInstance::FocusControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool FocusControllerInstance::init(utility::ErrorState& errorState)
	{
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "%s: missing KeyInputComponent", mID.c_str()))
			return false;

		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		mQuiltCameraComponent = getEntityInstance()->findComponent<QuiltCameraComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing QuiltCameraComponent", mID.c_str()))
			return false;

		pointer_component->pressed.connect(std::bind(&FocusControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&FocusControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&FocusControllerInstance::onMouseUp, this, std::placeholders::_1));

		// Connect key handlers
		key_component->pressed.connect(std::bind(&FocusControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&FocusControllerInstance::onKeyRelease, this, std::placeholders::_1));

		// Store reference to the transform specified in the resource
		mCurrentLookAtTarget = &(*mLookAtTargetComponent);
		
		FocusController* resource = getComponent<FocusController>();
		setAutoFocus(resource->mAutoFocus);

		enable();
		focus();

		return true;
	}


	void FocusControllerInstance::update(double deltaTime)
	{
		FocusController* resource = getComponent<FocusController>();

		float movement = resource->mMovementSpeed * deltaTime;

		glm::vec3 side{ 1.0f, 0.0f, 0.0f };
		glm::vec3 up{ 0.0f, 1.0f, 0.0f };
		glm::vec3 forward{ 0.0f, 0.0f, 1.0f };

		glm::vec3 dir_forward = glm::rotate(mTransformComponent->getRotate(), forward);
		glm::vec3 movement_forward = dir_forward * movement;

		glm::vec3 dir_sideways = glm::rotate(mTransformComponent->getRotate(), side);
		glm::vec3 movement_sideways = dir_sideways * movement;

		glm::vec3 movement_up = up * movement;

		if (mMoveForward)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() - movement_forward);
		}
		if (mMoveBackward)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + movement_forward);
		}
		if (mMoveLeft)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() - movement_sideways);
		}
		if (mMoveRight)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + movement_sideways);
		}
		if (mMoveUp)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + movement_up);
		}
		if (mMoveDown)
		{
			mTransformComponent->setTranslate(mTransformComponent->getTranslate() - movement_up);
		}
	}


	CameraComponentInstance& FocusControllerInstance::getCameraComponent()
	{
		return *mQuiltCameraComponent;
	}


	void FocusControllerInstance::focus()
	{
		// Increase/decrease distance to target
		float focus_distance = mQuiltCameraComponent->getDistanceToFocalPlane();
		const glm::vec3& direction = glm::normalize(mTransformComponent->getLocalTransform()[2]);

		auto lookat_position = math::extractPosition(mCurrentLookAtTarget->getGlobalTransform());
		mTransformComponent->setTranslate(lookat_position + direction * focus_distance);
	}


	void FocusControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		switch (pointerPressEvent.mButton)
		{
		case EMouseButton::LEFT:
			mMode = EMode::Rotating;
			break;
		case EMouseButton::RIGHT:
			mMode = EMode::Zooming;
			break;
		case EMouseButton::MIDDLE:
			mMode = EMode::Focus;
			break;
		default:
			break;
		}

		if (!mAutoFocus && mMode == EMode::Focus)
		{
			focus();
		}
	}


	void FocusControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		if (!mEnabled)
			return;

		mMode = EMode::Idle;
	}


	void FocusControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (!mEnabled)
			return;

		FocusController* resource = getComponent<FocusController>();

		auto focal_position = getFocalPosition();
		if (mMode == EMode::Rotating)
		{
			// We are using the relative movement of the mouse to update the camera
			float yaw = -(pointerMoveEvent.mRelX)  * getComponent<FocusController>()->mRotateSpeed;
			float pitch = pointerMoveEvent.mRelY * getComponent<FocusController>()->mRotateSpeed;

			// We need to rotate around the target point. We always first rotate around the local X axis (pitch), and then
			// we rotate around the y axis (yaw).
			glm::mat4 yaw_rotation = glm::rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 right = mTransformComponent->getLocalTransform()[0];
			glm::mat4 pitch_rotation = glm::rotate(pitch, glm::vec3(right.x, right.y, right.z));

			// To rotate around the target point, we take the current transform, then bring it into local target space (only translation), then first rotate pitch,
			// then rotate yaw, and then bring it back to worldspace.
			glm::mat4 transform = glm::translate(focal_position) * yaw_rotation * pitch_rotation * glm::translate(-focal_position) * mTransformComponent->getLocalTransform();

			// Our transform class always has the same sequence of applying translate, rotate, scale, so it can only rotate around it's own pivot. Therefore, we take
			// the worldtransform that we calculated and apply it backwards.
			glm::quat rotate = glm::normalize(glm::quat_cast(transform));
			glm::vec3 translate = math::extractPosition(transform);

			mTransformComponent->setTranslate(translate);
			mTransformComponent->setRotate(rotate);
		}
		else if (mMode == EMode::Zooming)
		{
			int pointer_move = pointerMoveEvent.mRelX;
			// Zooming should work on both x and y axes
			if (abs(pointerMoveEvent.mRelY) > abs(pointerMoveEvent.mRelX))
				pointer_move = pointerMoveEvent.mRelY;

			// Increase/decrease distance to target
			float distance = pointer_move * resource->mZoomSpeed;

			if (mAutoFocus)
			{
				// Pointer distance in pixel coordinates can be a bit harsh on the zoom movement,
				// therefore we multiply by a coefficient to make it less so.
				const float pointer_distance_mult = 1/16.f;
				(*mQuiltCameraComponent).setCameraSize((*mQuiltCameraComponent).getCameraSize() + distance * pointer_distance_mult);
				focus();
			}
			else
			{
				const glm::vec3& direction = mTransformComponent->getLocalTransform()[2];
				const glm::vec3& translate = mTransformComponent->getLocalTransform()[3];
				glm::vec3 new_translate = translate - direction * distance;

				// Ensure the look direction does not flip
				const glm::vec3 lookdir_prevframe = focal_position - translate;
				const glm::vec3 lookdir1_curframe = focal_position - new_translate;
				if (glm::dot(lookdir_prevframe, lookdir1_curframe) < 0.0f)
					return;

				// Ensure the distance from the target does not exceed the specified minimum
				if (glm::length(new_translate) < resource->mMinZoomDistance)
					new_translate = glm::normalize(new_translate) * resource->mMinZoomDistance;

				mTransformComponent->setTranslate(new_translate);
			}
		}
	}


	void FocusControllerInstance::onKeyPress(const KeyPressEvent& keyPressEvent)
	{
		switch (keyPressEvent.mKey)
		{
		case EKeyCode::KEY_w:
		{
			mMoveForward = true;
			break;
		}
		case EKeyCode::KEY_s:
		{
			mMoveBackward = true;
			break;
		}
		case EKeyCode::KEY_a:
		{
			mMoveLeft = true;
			break;
		}
		case EKeyCode::KEY_d:
		{
			mMoveRight = true;
			break;
		}
		case EKeyCode::KEY_q:
		{
			mMoveDown = true;
			break;
		}
		case EKeyCode::KEY_e:
		{
			mMoveUp = true;
			break;
		}
		}
	}


	void FocusControllerInstance::onKeyRelease(const KeyReleaseEvent& keyReleaseEvent)
	{
		switch (keyReleaseEvent.mKey)
		{
		case EKeyCode::KEY_w:
		{
			mMoveForward = false;
			break;
		}
		case EKeyCode::KEY_s:
		{
			mMoveBackward = false;
			break;
		}
		case EKeyCode::KEY_a:
		{
			mMoveLeft = false;
			break;
		}
		case EKeyCode::KEY_d:
		{
			mMoveRight = false;
			break;
		}
		case EKeyCode::KEY_q:
		{
			mMoveDown = false;
			break;
		}
		case EKeyCode::KEY_e:
		{
			mMoveUp = false;
			break;
		}
		}
	}


	glm::vec3 FocusControllerInstance::getFocalPosition() const
	{
		return math::extractPosition((*mLookAtTargetComponent).getGlobalTransform());
	}


	void FocusControllerInstance::setLookAtTarget(TransformComponentInstance& transform)
	{
		mCurrentLookAtTarget = &transform;
	}


	void FocusController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(QuiltCameraComponent));
		components.push_back(RTTI_OF(TransformComponent));
		components.push_back(RTTI_OF(KeyInputComponent));
		components.push_back(RTTI_OF(PointerInputComponent));
	}
}
