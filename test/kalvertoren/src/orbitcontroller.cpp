#include "orbitcontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <nap/entity.h>
#include <mathutils.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "glm/gtx/orthonormalize.hpp"

RTTI_BEGIN_CLASS(nap::OrbitController)
	RTTI_PROPERTY("MovementSpeed", &nap::OrbitController::mMovementSpeed, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateSpeed", &nap::OrbitController::mRotateSpeed, nap::rtti::EPropertyMetaData::Default)
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


	bool OrbitControllerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "Could not find PointerInputComponent"))
			return false;

		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "Could not find transform component"))
			return false;

		pointer_component->pressed.connect(std::bind(&OrbitControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&OrbitControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&OrbitControllerInstance::onMouseUp, this, std::placeholders::_1));

		return true;
	}


	void OrbitControllerInstance::startDrag()
	{
		glm::vec3 translate = mTransformComponent->getLocalTransform()[3];
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::mat4 rotation = glm::lookAt(translate, mLookAtPos, up);
		rotation = glm::inverse(rotation);
		mTransformComponent->setRotate(rotation);
	}


	void OrbitControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		if (pointerPressEvent.mButton == EMouseButton::LEFT)
		{
			startDrag();
			mMode = EMode::Rotating;
		}
		else if (pointerPressEvent.mButton == EMouseButton::RIGHT)
		{
			startDrag();
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
			float yaw = -(pointerMoveEvent.mRelX)  * getComponent<OrbitController>()->mRotateSpeed;
			float pitch = -(pointerMoveEvent.mRelY) * getComponent<OrbitController>()->mRotateSpeed;

			glm::mat4 yaw_rotation = glm::rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::vec4 right = mTransformComponent->getLocalTransform()[0];
			glm::mat4 pitch_rotation = glm::rotate(pitch, glm::vec3(right.x, right.y, right.z));

			glm::mat4 transform = glm::translate(mLookAtPos) * yaw_rotation * pitch_rotation * glm::translate(-mLookAtPos) * mTransformComponent->getLocalTransform();

			glm::quat rotate = glm::quat_cast(transform);
			rotate = glm::normalize(rotate);
			glm::vec4 translate = transform[3];

			mTransformComponent->setTranslate(translate);
			mTransformComponent->setRotate(rotate);
		}
		else if (mMode == EMode::Zooming)
		{
			float distance = pointerMoveEvent.mRelY * getComponent<OrbitController>()->mMovementSpeed;

			const glm::vec3& direction = mTransformComponent->getLocalTransform()[2];
			const glm::vec3& translate = mTransformComponent->getLocalTransform()[3];

			mTransformComponent->setTranslate(translate - direction * distance);
		}
	}
}