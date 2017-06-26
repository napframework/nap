#pragma once

#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include "nap/entity.h"
#include <napinputevent.h>

namespace nap
{
	// Forward declares
	class InputService;

	class InputComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		InputComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		virtual void trigger(const nap::InputEvent& inEvent) = 0;
	};


	class InputComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)
	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(InputComponent); }
	};


	/**
	@brief Button Input Component

	Handles key presses (event type keypress)
	**/
	class KeyInputComponent : public InputComponent
	{
		friend class InputService;
		RTTI_ENABLE(InputComponent)
	
	public:
		KeyInputComponent(EntityInstance& entity) :
			InputComponent(entity)
		{
		}

		// Signals
		Signal<const KeyPressEvent&>		pressed;		//< If the key has been pressed
		Signal<const KeyReleaseEvent&>		released;		//< If the key has been released

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};

	class KeyInputComponentResource : public InputComponentResource
	{
		RTTI_ENABLE(InputComponentResource)

	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(KeyInputComponent); }
	};

	/**
	@brief pointer input component

	Handles pointer (mouse) presses input events
	**/
	class PointerInputComponent : public InputComponent
	{
		friend class InputService;
		RTTI_ENABLE(InputComponent)

	public:
		PointerInputComponent(EntityInstance& entity) :
			InputComponent(entity)
		{
		}

		Signal<const PointerPressEvent&>	pressed;		//< If the input component was clicked
		Signal<const PointerReleaseEvent&>	released;		//< If the input component click has been released
		Signal<const PointerDragEvent&>		dragged;		//< If the component received a drag (mousedrag)
		Signal<const PointerMoveEvent&>		moved;			//< If the component received a move (mousemove)

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};

	class PointerInputComponentResource : public InputComponentResource
	{
		RTTI_ENABLE(InputComponentResource)

	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(PointerInputComponent); }
	};

}