#include <inputcomponent.h>
#include <nap/entity.h>

RTTI_BEGIN_BASE_CLASS(nap::InputComponent)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::InputComponentInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::KeyInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::KeyInputComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PointerInputComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::PointerInputComponentInstance, nap::EntityInstance&)
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
		else if (event_type == RTTI_OF(PointerDragEvent))
		{
			const PointerDragEvent& drag_event = static_cast<const PointerDragEvent&>(inEvent);
			dragged.trigger(drag_event);
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

		if (event_type == RTTI_OF(KeyReleaseEvent))
		{
			const KeyReleaseEvent& release_event = static_cast<const KeyReleaseEvent&>(inEvent);
			released.trigger(release_event);
			return;
		}
	}
}