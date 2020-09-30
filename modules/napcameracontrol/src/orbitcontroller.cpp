/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "orbitcontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <entity.h>
#include <mathutils.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "glm/gtx/orthonormalize.hpp"

RTTI_BEGIN_CLASS(nap::OrbitController)
	RTTI_PROPERTY("MovementSpeed",			&nap::OrbitController::mMovementSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateSpeed",			&nap::OrbitController::mRotateSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PerspCameraComponent",	&nap::OrbitController::mPerspCameraComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LookAtPosition",			&nap::OrbitController::mLookAtPos,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrbitControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	OrbitControllerInstance::OrbitControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool OrbitControllerInstance::init(utility::ErrorState& errorState)
	{
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		pointer_component->pressed.connect(std::bind(&OrbitControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&OrbitControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&OrbitControllerInstance::onMouseUp, this, std::placeholders::_1));

		enable(getComponent<OrbitController>()->mLookAtPos);

		return true;
	}


	CameraComponentInstance& OrbitControllerInstance::getCameraComponent()
	{
		return *mPerspCameraComponent;
	}


	void OrbitControllerInstance::enable(const glm::vec3& cameraPos, const glm::vec3& lookAtPos)
	{
		if (!mEnabled)
		{
			// Construct a lookat matrix. Note that if this is currently called when facing up or downward, the
			// camera may flip around the y axis. Currently this is only called when starting orbit so it isn't
			// much of a problem, but if it is, we need to find another way of constructing a lookat camera.
			glm::vec3 up(0.0f, 1.0f, 0.0f);
			glm::mat4 rotation = glm::lookAt(cameraPos, lookAtPos, up);
			rotation = glm::inverse(rotation);
			mTransformComponent->setRotate(rotation);
			mLookAtPos = lookAtPos;
			mEnabled = true;
		}
	}


	void OrbitControllerInstance::enable(const glm::vec3& lookAtPos)
	{
		if (!mEnabled)
		{
			glm::vec3 translate = mTransformComponent->getLocalTransform()[3];
			enable(translate, lookAtPos);
		}
	}


	void OrbitControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		if (pointerPressEvent.mButton == EMouseButton::LEFT)
		{
			mMode = EMode::Rotating;
		}
		else if (pointerPressEvent.mButton == EMouseButton::RIGHT)
		{
			mMode = EMode::Zooming;
		}
	}


	void OrbitControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		mMode = EMode::Idle;
	}


	void OrbitControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (mMode == EMode::Rotating)
		{
			// We are using the relative movement of the mouse to update the camera
			float yaw = -(pointerMoveEvent.mRelX)  * getComponent<OrbitController>()->mRotateSpeed;
			float pitch = pointerMoveEvent.mRelY * getComponent<OrbitController>()->mRotateSpeed;

			// We need to rotate around the target point. We always first rotate around the local X axis (pitch), and then
			// we rotate around the y axis (yaw).
			glm::mat4 yaw_rotation = glm::rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 right = mTransformComponent->getLocalTransform()[0];
			glm::mat4 pitch_rotation = glm::rotate(pitch, glm::vec3(right.x, right.y, right.z));

			// To rotate around the target point, we take the current transform, then bring it into local target space (only translation), then first rotate pitch,
			// then rotate yaw, and then bring it back to worldspace.
			glm::mat4 transform = glm::translate(mLookAtPos) * yaw_rotation * pitch_rotation * glm::translate(-mLookAtPos) * mTransformComponent->getLocalTransform();

			// Our transform class always has the same sequence of applying translate, rotate, scale, so it can only rotate around it's own pivot. Therefore, we take
			// the worldtransform that we calculated and apply it backwards.
			glm::quat rotate = glm::quat_cast(transform);
			rotate = glm::normalize(rotate);
			glm::vec4 translate = transform[3];

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
			float distance = pointer_move * getComponent<OrbitController>()->mMovementSpeed;
			const glm::vec3& direction = mTransformComponent->getLocalTransform()[2];
			const glm::vec3& translate = mTransformComponent->getLocalTransform()[3];

			mTransformComponent->setTranslate(translate - direction * distance);
		}
	}


	void OrbitController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
		components.push_back(RTTI_OF(KeyInputComponent));
	}

}