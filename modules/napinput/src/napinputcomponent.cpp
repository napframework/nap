#include <napinputcomponent.h>

#include <nap/objectutils.h>

RTTI_BEGIN_BASE_CLASS(nap::InputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::KeyInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PointerInputComponent)
RTTI_END_CLASS


namespace nap
{
	// Pointer input forwarding
	void nap::PointerInputComponent::trigger(nap::InputEvent& inEvent)
	{
		// Make sure it's a pointer event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(PointerEvent)))
		{
			nap::Logger::warn("Received a non pointer event in a pointer input event handler!");
			return;
		}
		
		// Forward to correct signal
		if (event_type == RTTI_OF(PointerPressEvent))
		{
			PointerPressEvent& press_event = static_cast<PointerPressEvent&>(inEvent);
			pressed.trigger(press_event);

		}
		else if (event_type == RTTI_OF(PointerReleaseEvent))
		{
			PointerReleaseEvent& release_event = static_cast<PointerReleaseEvent&>(inEvent);
			released.trigger(release_event);
		}
		else if (event_type == RTTI_OF(PointerMoveEvent))
		{
			PointerMoveEvent& move_event = static_cast<PointerMoveEvent&>(inEvent);
			moved.trigger(move_event);
			return;
		}
		else if (event_type == RTTI_OF(PointerDragEvent))
		{
			PointerDragEvent& drag_event = static_cast<PointerDragEvent&>(inEvent);
			dragged.trigger(drag_event);
			return;
		}
		else
		{
			nap::Logger::warn("unable to find matching input handler for pointer event of type: %s", inEvent.get_type().get_name().data());
		}
	}


	// Key forward handling
	void KeyInputComponent::trigger(nap::InputEvent& inEvent)
	{
		// Make sure it's a pointer event
		rtti::TypeInfo event_type = inEvent.get_type().get_raw_type();
		if (!event_type.is_derived_from(RTTI_OF(KeyEvent)))
		{
			nap::Logger::warn("Received a non key event in a key input event handler!");
			return;
		}

		if (event_type == RTTI_OF(KeyPressEvent))
		{
			KeyPressEvent& press_event = static_cast<KeyPressEvent&>(inEvent);
			pressed.trigger(press_event);
			return;
		}

		if (event_type == RTTI_OF(KeyReleaseEvent))
		{
			KeyReleaseEvent& release_event = static_cast<KeyReleaseEvent&>(inEvent);
			released.trigger(release_event);
			return;
		}

		nap::Logger::warn("unable to find matching input handler for key event of type: %s", inEvent.get_type().get_name().data());
	}
}