/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "firstpersoncontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <entity.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

RTTI_BEGIN_CLASS(nap::FirstPersonController)
	RTTI_PROPERTY("MovementSpeed",			&nap::FirstPersonController::mMovementSpeed,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateSpeed",			&nap::FirstPersonController::mRotateSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PerspCameraComponent",	&nap::FirstPersonController::mPerspCameraComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS 

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FirstPersonControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	FirstPersonControllerInstance::FirstPersonControllerInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
	{
	}


	bool FirstPersonControllerInstance::init(utility::ErrorState& errorState)
	{
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		// KeyInputComponent is required to receive input
		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "%s: missing KeyInputComponent", mID.c_str()))
			return false;

		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Connect mouse handlers
		pointer_component->pressed.connect(std::bind(&FirstPersonControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&FirstPersonControllerInstance::onMouseUp, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&FirstPersonControllerInstance::onMouseMove, this, std::placeholders::_1));

		// Connect key handlers
		key_component->pressed.connect(std::bind(&FirstPersonControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&FirstPersonControllerInstance::onKeyRelease, this, std::placeholders::_1));

		// Copy properties
		mMovementSpeed = getComponent<FirstPersonController>()->mMovementSpeed;
		mRotateSpeed = getComponent<FirstPersonController>()->mRotateSpeed;

		return true;
	}


	void FirstPersonControllerInstance::enable(const glm::vec3& translate, const glm::quat& rotate)
	{
		mTransformComponent->setTranslate(translate);
		mTransformComponent->setRotate(rotate);
		mEnabled = true;
	}


	void FirstPersonControllerInstance::enable()
	{
		mEnabled = true;
	}


	CameraComponentInstance& FirstPersonControllerInstance::getCameraComponent()
	{
		return *mPerspCameraComponent;
	}


	void FirstPersonControllerInstance::update(double deltaTime)
	{
		if (!mEnabled)
			return;

		float movement = mMovementSpeed * deltaTime;

		glm::vec3 side(1.0, 0.0, 0.0);
		glm::vec3 up(0.0, 1.0, 0.0);
		glm::vec3 forward(0.0, 0.0, 1.0);

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


	void FirstPersonControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		// Enable movement when lmb is pressed
		if (pointerPressEvent.mButton == EMouseButton::LEFT)
			mMoving = true;
	}
	

	void FirstPersonControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		if (pointerReleaseEvent.mButton == EMouseButton::LEFT)
			mMoving = false;
	}


	void FirstPersonControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (!mMoving)
			return; 

		float yaw = -(pointerMoveEvent.mRelX  * mRotateSpeed);
		float pitch = pointerMoveEvent.mRelY  * mRotateSpeed;

		glm::mat4 yaw_rotation = glm::rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec4 right = mTransformComponent->getLocalTransform()[0];
		glm::mat4 pitch_rotation = glm::rotate(pitch, glm::vec3(right.x, right.y, right.z));

		mTransformComponent->setRotate(yaw_rotation * pitch_rotation * glm::toMat4(mTransformComponent->getRotate()));
	}


	void FirstPersonControllerInstance::onKeyPress(const KeyPressEvent& keyPressEvent)
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


	void FirstPersonControllerInstance::onKeyRelease(const KeyReleaseEvent& keyReleaseEvent)
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


	void FirstPersonController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
		components.emplace_back(RTTI_OF(KeyInputComponent));
	}

}
