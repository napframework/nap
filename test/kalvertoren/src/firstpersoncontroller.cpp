#include "firstpersoncontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <nap/entity.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

RTTI_BEGIN_CLASS(nap::FirstPersonController)
	RTTI_PROPERTY("MovementSpeed",	&nap::FirstPersonController::mMovementSpeed,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotateSpeed",	&nap::FirstPersonController::mRotateSpeed,		nap::rtti::EPropertyMetaData::Default)
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


	bool FirstPersonControllerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// KeyInputComponent is required to receive input
		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "Could not find KeyInputComponent"))
			return false;

		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "Could not find transform component"))
			return false;

		// Connect key handlers
		key_component->pressed.connect(std::bind(&FirstPersonControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&FirstPersonControllerInstance::onKeyRelease, this, std::placeholders::_1));

		return true;
	}


	void FirstPersonControllerInstance::update(double deltaTime)
	{
		FirstPersonController* resource = getComponent<FirstPersonController>();

		float movement = resource->mMovementSpeed * deltaTime;
		float rotate = resource->mRotateSpeed * deltaTime;
		float rotate_rad = rotate;

		glm::vec3 side(1.0, 0.0, 0.0);
		glm::vec3 forward(0.0, 0.0, 1.0);

		glm::vec3 dir_forward = glm::rotate(mTransformComponent->getRotate(), forward);
		glm::vec3 movement_forward = dir_forward * movement;

		glm::vec3 dir_sideways = glm::rotate(mTransformComponent->getRotate(), side);
		glm::vec3 movement_sideways = dir_sideways * movement;

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
		if (mLookUp)
		{
			glm::quat r = mTransformComponent->getRotate();
			glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(1.0, 0.0, 0.0));
			mTransformComponent->setRotate(nr);
		}
		if (mLookDown)
		{
			glm::quat r = mTransformComponent->getRotate();
			glm::quat nr = glm::rotate(r, -1.0f * rotate_rad, glm::vec3(1.0, 0.0, 0.0));
			mTransformComponent->setRotate(nr);
		}
		if (mLookRight)
		{
			glm::quat r = mTransformComponent->getRotate();
			glm::quat nr = glm::rotate(r, -1.0f*rotate_rad, glm::vec3(0.0, 1.0, 0.0));
			mTransformComponent->setRotate(nr);
		}
		if (mLookLeft)
		{
			glm::quat r = mTransformComponent->getRotate();
			glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(0.0, 1.0, 0.0));
			mTransformComponent->setRotate(nr);
		}
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
			case EKeyCode::KEY_UP:
			{
				mLookUp = true;
				break;
			}
			case EKeyCode::KEY_DOWN:
			{
				mLookDown = true;
				break;
			}
			case EKeyCode::KEY_LEFT:
			{
				mLookLeft = true;
				break;
			}
			case EKeyCode::KEY_RIGHT:
			{
				mLookRight = true;
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
			case EKeyCode::KEY_UP:
			{
				mLookUp = false;
				break;
			}
			case EKeyCode::KEY_DOWN:
			{
				mLookDown = false;
				break;
			}
			case EKeyCode::KEY_LEFT:
			{
				mLookLeft = false;
				break;
			}
			case EKeyCode::KEY_RIGHT:
			{
				mLookRight = false;
				break;
			}
		}
	}
}