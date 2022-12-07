/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

RTTI_BEGIN_CLASS(nap::TouchInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchInputComponentInstance)
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
		// Ensure it's a pointer event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(nap::PointerEvent)))
			return;

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


	// Touch input forwarding
	void TouchInputComponentInstance::trigger(const nap::InputEvent& inEvent)
	{
		// Ensure it's a touch event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(nap::TouchEvent)))
			return;

		if (event_type == RTTI_OF(PointerPressEvent))
		{
			const TouchPressEvent& press_event = static_cast<const TouchPressEvent&>(inEvent);
			pressed.trigger(press_event);

		}
		else if (event_type == RTTI_OF(PointerReleaseEvent))
		{
			const TouchReleaseEvent& release_event = static_cast<const TouchReleaseEvent&>(inEvent);
			released.trigger(release_event);
		}
		else if (event_type == RTTI_OF(PointerMoveEvent))
		{
			const TouchMoveEvent& move_event = static_cast<const TouchMoveEvent&>(inEvent);
			moved.trigger(move_event);
			return;
		}
	}


	// Key forward handling
	void KeyInputComponentInstance::trigger(const nap::InputEvent& inEvent)
	{
		// Ensure it's a key event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(nap::KeyEvent)))
			return;

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
		// Ensure it's a controller event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(nap::ControllerEvent)))
			return;

		if (event_type == RTTI_OF(nap::ControllerButtonPressEvent))
		{
			const ControllerButtonPressEvent& press_event = static_cast<const ControllerButtonPressEvent&>(inEvent);
			pressed.trigger(press_event);
		}

		else if (event_type == RTTI_OF(nap::ControllerButtonReleaseEvent))
		{
			const ControllerButtonReleaseEvent& release_event = static_cast<const ControllerButtonReleaseEvent&>(inEvent);
			released.trigger(release_event);
		}

		else if (event_type == RTTI_OF(nap::ControllerAxisEvent))
		{
			const ControllerAxisEvent& axis_event = static_cast<const ControllerAxisEvent&>(inEvent);
			axisChanged.trigger(axis_event);
		}
	}
}
