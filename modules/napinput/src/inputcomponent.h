#pragma once

#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include <nap/componentinstance.h>
#include <inputevent.h>
#include <nap/dllexport.h>

namespace nap
{
	// Forward declares
	class InputService;

	/**
	 * Base class for all input components. The trigger function is called when an InputRouter-derived class
	 * decides to route the input to this specific component.
	 */
	class NAPAPI InputComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		InputComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		/**
		 * This function is called by an InputRouter derived class if it decides to route the input to this component.
		 * Implement this function in a derived class to handle input. 
		 * @param inEvent An InputEvent-derived class. Use RTTI queries to test the type of event and to retrieve data from it.
		 */
		virtual void trigger(const nap::InputEvent& inEvent) = 0;
	};


	/**
	 * The resource class for the InputComponent.
	 */
	class InputComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)
	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(InputComponent); }
	};



	/**
	 * Input component for press/release key events.
	 */
	class NAPAPI KeyInputComponent : public InputComponent
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


	/**
 	 * Resource class for KeyInputComponent.
	 */
	class NAPAPI KeyInputComponentResource : public InputComponentResource
	{
		RTTI_ENABLE(InputComponentResource)

	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(KeyInputComponent); }
	};


	/**
	 * Input component for mouse/touch events.
	 */
	class NAPAPI PointerInputComponent : public InputComponent
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


	/**
	 * Resource class for PointerInputComponent.
	 */
	class NAPAPI PointerInputComponentResource : public InputComponentResource
	{
		RTTI_ENABLE(InputComponentResource)

	public:
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(PointerInputComponent); }
	};

}