#pragma once

#include <rtti/rtti.h>
#include <nap/serviceablecomponent.h>
#include <nap/signalslot.h>
#include <nap/eventdispatcher.h>
#include <napinputevent.h>

namespace nap
{
	// Forward declares
	class InputService;

	/**
	@brief InputComponent

	Defines a component that can be called by the input service when an input event occurs
	Register slots to the signals exposed to this component to define behavior
	**/

	class InputComponent : public ServiceableComponent
	{
		friend class InputService;
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

	public:
		// Default constructor
		InputComponent() = default;

		// Disable copy
		InputComponent(const InputComponent& that) = delete;
		InputComponent& operator=(const InputComponent&) = delete;

		// If this input component is input enabled
		Attribute<bool> enabled = { this, "enabled", true };

	protected:
		virtual void trigger(nap::InputEvent& inEvent) = 0;
	};


	/**
	@brief Button Input Component

	Handles key presses (event type keypress)
	**/
	class KeyInputComponent : public InputComponent
	{
		friend class InputService;
		RTTI_ENABLE_DERIVED_FROM(InputComponent)
	
	public:
		// Signals
		Signal<KeyPressEvent&>			pressed;		//< If the key has been pressed
		Signal<KeyReleaseEvent&>		released;		//< If the key has been released

	protected:
		virtual void trigger(nap::InputEvent& inEvent) override;
	};


	/**
	@brief pointer input component

	Handles pointer (mouse) presses input events
	**/
	class PointerInputComponent : public InputComponent
	{
		friend class InputService;
		RTTI_ENABLE_DERIVED_FROM(InputComponent)
	public:
		Signal<PointerPressEvent&>		pressed;		//< If the input component was clicked
		Signal<PointerReleaseEvent&>	released;		//< If the input component click has been released
		Signal<PointerDragEvent&>		dragged;		//< If the component received a drag (mousedrag)
		Signal<PointerMoveEvent&>		moved;			//< If the component received a move (mousemove)

	protected:
		EventDispatcher mEventDispatcher = { this, "EventDispatcher" };
		virtual void trigger(nap::InputEvent& inEvent) override;
	};
}

RTTI_DECLARE_BASE(nap::InputComponent)
RTTI_DECLARE(nap::KeyInputComponent)
RTTI_DECLARE(nap::PointerInputComponent)