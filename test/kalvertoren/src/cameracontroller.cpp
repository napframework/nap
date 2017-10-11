#include "cameracontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <nap/entity.h>

RTTI_BEGIN_CLASS(nap::CameraController)
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


	bool CameraControllerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// KeyInputComponent is required to receive input
		KeyInputComponentInstance* key_component = getEntityInstance()->findComponent<KeyInputComponentInstance>();
		if (!errorState.check(key_component != nullptr, "Could not find KeyInputComponent"))
			return false;

		mOrbitComponent = getEntityInstance()->findComponent<OrbitControllerInstance>();
		if (!errorState.check(mOrbitComponent != nullptr, "Could not find OrbitControllerInstance"))
			return false;

		mFirstPersonComponent = getEntityInstance()->findComponent<FirstPersonControllerInstance>();
		if (!errorState.check(mFirstPersonComponent != nullptr, "Could not find FirstPersonControllerInstance"))
			return false;

		key_component->pressed.connect(std::bind(&CameraControllerInstance::onKeyPress, this, std::placeholders::_1));
		key_component->released.connect(std::bind(&CameraControllerInstance::onKeyRelease, this, std::placeholders::_1));

		return true; 
	}

	void CameraControllerInstance::onKeyPress(const KeyPressEvent& keyReleaseEvent)
	{
		switch (keyReleaseEvent.mKey)
		{
		case EKeyCode::KEY_LALT:
			mFirstPersonComponent->enable(false);
			mOrbitComponent->enable(true);
			mMode = EMode::Orbit;
		}
	}

	void CameraControllerInstance::onKeyRelease(const KeyReleaseEvent& keyReleaseEvent)
	{
		switch (keyReleaseEvent.mKey)
		{
		case EKeyCode::KEY_LALT:
			mFirstPersonComponent->enable(true);
			mOrbitComponent->enable(false);
			mMode = EMode::FirstPerson;
			break;
		}
	}
}