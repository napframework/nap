#include "pathcontroller.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <inputcomponent.h>
#include <lineutils.h>
#include <nap/logger.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::pathcontroller run time class definition 
RTTI_BEGIN_CLASS(nap::PathController)
	RTTI_PROPERTY("Path",			&nap::PathController::mPath,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathTransform",	&nap::PathController::mPathTransform,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position",		&nap::PathController::mPosition,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MovementSpeed",	&nap::PathController::mMovementSpeed,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::pathcontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PathControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void PathController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
		components.emplace_back(RTTI_OF(KeyInputComponent));
	}


	bool PathControllerInstance::init(utility::ErrorState& errorState)
	{
		// KeyInputComponent is required to receive input
		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "%s: missing key input component", mID.c_str()))
			return false;

		// TransformComponent is required to move the entity
		mCameraTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mCameraTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Connect key handlers
		key_component->pressed.connect(std::bind(&PathControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&PathControllerInstance::onKeyRelease, this, std::placeholders::_1));

		// Copy over properties
		mSpeed = getComponent<PathController>()->mMovementSpeed;
		mPosition = getComponent<PathController>()->mPosition;

		// Get distances along line
		mPath = getComponent<PathController>()->mPath.get();
		mPath->getDistances(mLineDistances);

		return true;
	}


	void PathControllerInstance::update(double deltaTime)
	{
		// Calculate new position
		mPosition = fmod(mPosition + (deltaTime * mSpeed), 1.0f);

		glm::vec3 new_pos;
		math::getValueAlongLine<glm::vec3>(mLineDistances, mPath->getPositionAttr().mData, mPosition, new_pos);
		glm::mat4 new_camera_xform = glm::translate(mLineTransform->getGlobalTransform(), new_pos);
		mCameraTransform->setTranslate(new_camera_xform[3]);
	}


	void PathControllerInstance::enable()
	{
		mEnabled = true;
	}


	void PathControllerInstance::enable(float position)
	{
		mEnabled = true;
		mPosition = position;
	}


	void PathControllerInstance::disable()
	{
		mEnabled = false;
	}


	void PathControllerInstance::onKeyPress(const KeyPressEvent& keyPressEvent)
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
		}
	}


	void PathControllerInstance::onKeyRelease(const KeyReleaseEvent& keyReleaseEvent)
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
		}
	}

}