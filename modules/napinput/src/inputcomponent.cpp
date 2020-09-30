#include <inputcomponent.h>
#include <entity.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InputComponentInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::KeyInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KeyInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PointerInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointerInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ControllerInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControllerInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS



namespace nap
{
	// Pointer input forwarding
	void nap::PointerInputComponentInstance::trigger(const nap::InputEvent& inEvent)
	{
		// Make sure it's a pointer event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		
		// Forward to correct signal
		if (event_type == RTTI_OF(PointerPressEvent))
		{
			const PointerPressEvent& press_event = static_cast<const PointerPressEvent&>(inEvent);
			pressed.trigger(press_event);

		}
		else if (event_type == RTTI_OF(PointerReleaseEvent))
		{
			const PointerReleaseEvent& release_event = static_cast<const PointerReleaseEvent&>(inEvent);
			released.trigger(release_event);
		}
		else if (event_type == RTTI_OF(PointerMoveEvent))
		{
			const PointerMoveEvent& move_event = static_cast<const PointerMoveEvent&>(inEvent);
			moved.trigger(move_event);
			return;
		}
	}


	// Key forward handling
	void KeyInputComponentInstance::trigger(const nap::InputEvent& inEvent)
	{
		// Make sure it's a pointer event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();

		if (event_type == RTTI_OF(KeyPressEvent))
		{
			const KeyPressEvent& press_event = static_cast<const KeyPressEvent&>(inEvent);
			pressed.trigger(press_event);
			return;
		}

		else if (event_type == RTTI_OF(KeyReleaseEvent))
		{
			const KeyReleaseEvent& release_event = static_cast<const KeyReleaseEvent&>(inEvent);
			released.trigger(release_event);
			return;
		}
	}


	// Controller input forward handling
	void ControllerInputComponentInstance::trigger(const nap::InputEvent& inEvent)
	{
		// Make sure it's a controller event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();

		if (event_type == RTTI_OF(nap::ControllerButtonPressEvent))
		{
			const ControllerButtonPressEvent& press_event = static_cast<const ControllerButtonPressEvent&>(inEvent);
			pressed.trigger(press_event);
			Logger::info("button pressed: %d", (int)(press_event.mButton));
		}

		else if (event_type == RTTI_OF(nap::ControllerButtonReleaseEvent))
		{
			const ControllerButtonReleaseEvent& release_event = static_cast<const ControllerButtonReleaseEvent&>(inEvent);
			released.trigger(release_event);
			Logger::info("button released: %d", (int)(release_event.mButton));
		}

		else if (event_type == RTTI_OF(nap::ControllerAxisEvent))
		{
			const ControllerAxisEvent& axis_event = static_cast<const ControllerAxisEvent&>(inEvent);
			axisChanged.trigger(axis_event);
		}
	}

}