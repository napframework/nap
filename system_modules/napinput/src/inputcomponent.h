/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include <component.h>
#include <inputevent.h>
#include <utility/dllexport.h>

namespace nap
{
	// Forward declares
	class InputService;
	class InputComponentInstance;

	/**
	 * Base class for all input components that receive input events.
	 */
	class InputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(InputComponent, InputComponentInstance)
	};


	/**
	 * Base class for all input components. The trigger function is called when an InputRouter-derived class
	 * decides to route the input to this specific component.
	 */
	class NAPAPI InputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		InputComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{ }

		/**
		 * This function is called by an InputRouter derived class if it decides to route the input to this component.
		 * Implement this function in a derived class to handle input. 
		 * @param inEvent An InputEvent-derived class. Use RTTI queries to test the type of event and to retrieve data from it.
		 */
		virtual void trigger(const nap::InputEvent& inEvent) = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// Key Input Component
	//////////////////////////////////////////////////////////////////////////

	class KeyInputComponentInstance;

	/**
	 * Input component that receives keyboard press and release events. 
	 */
	class NAPAPI KeyInputComponent : public InputComponent
	{
		RTTI_ENABLE(InputComponent)
		DECLARE_COMPONENT(KeyInputComponent, KeyInputComponentInstance)
	};


	/**
	 * Input component that receives keyboard press and release events.
	 * Register to the various signals to receive key events.
	 */
	class NAPAPI KeyInputComponentInstance : public InputComponentInstance
	{
		friend class InputService;
		RTTI_ENABLE(InputComponentInstance)
	
	public:
		KeyInputComponentInstance(EntityInstance& entity, Component& resource) :
			InputComponentInstance(entity, resource)
		{ }

		// Signals
		Signal<const KeyPressEvent&>		pressed;		///< Signal emitted when a key is pressed
		Signal<const KeyReleaseEvent&>		released;		///< Signal emitted when a key is released

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Pointer Input Component
	//////////////////////////////////////////////////////////////////////////

	class PointerInputComponentInstance;

	/**
	 * Input component that receives mouse events.
	 */
	class NAPAPI PointerInputComponent : public InputComponent
	{
		RTTI_ENABLE(InputComponent)
			DECLARE_COMPONENT(PointerInputComponent, PointerInputComponentInstance)
	};


	/**
	 * Input component that receives mouse events
	 * Register to the various signals to receive mouse events
	 */
	class NAPAPI PointerInputComponentInstance : public InputComponentInstance
	{
		friend class InputService;
		RTTI_ENABLE(InputComponentInstance)

	public:
		PointerInputComponentInstance(EntityInstance& entity, Component& resource) :
			InputComponentInstance(entity, resource)
		{ }

		Signal<const PointerPressEvent&>	pressed;		///< Signal emitted when the input component receives a click
		Signal<const PointerReleaseEvent&>	released;		///< Signal emitted when the click is released
		Signal<const PointerMoveEvent&>		moved;			///< Signal emitted when this component receives a mouse move

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Touch Input Component
	//////////////////////////////////////////////////////////////////////////

	class TouchInputComponentInstance;

	/**
	 * Input component that receives touch input events. 
	 * Resource class for TouchInputComponent.
	 */
	class NAPAPI TouchInputComponent : public InputComponent
	{
		RTTI_ENABLE(InputComponent)
		DECLARE_COMPONENT(TouchInputComponent, TouchInputComponentInstance)
	};


	/**
	 * Touch event input component.
	 * Listen to the various signals to receive touch events.
	 */
	class NAPAPI TouchInputComponentInstance : public InputComponentInstance
	{
		friend class InputService;
		RTTI_ENABLE(InputComponentInstance)

	public:
		TouchInputComponentInstance(EntityInstance& entity, Component& resource) :
			InputComponentInstance(entity, resource)
		{ }

		Signal<const TouchPressEvent&>		pressed;		///< Signal emitted when the component receives a finger press
		Signal<const TouchReleaseEvent&>	released;		///< Signal emitted when the finger is released
		Signal<const TouchMoveEvent&>		moved;			///< Signal emitted when the finger moves

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Controller Input Component
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Resource class for ControllerInputComponent
	 * Receives joystick / controller input.
	 */
	class ControllerInputComponentInstance;
	class NAPAPI ControllerInputComponent : public InputComponent
	{
		RTTI_ENABLE(InputComponent)
			DECLARE_COMPONENT(ControllerInputComponent, ControllerInputComponentInstance)
	};


	/**
	 * Instance component for controller / joystick events
	 * Register to the various signals to receive controller / joystick events 
	 */
	class NAPAPI ControllerInputComponentInstance : public InputComponentInstance
	{
		friend class InputService;
		RTTI_ENABLE(InputComponentInstance)

	public:
		ControllerInputComponentInstance(EntityInstance& entity, Component& resource) :
			InputComponentInstance(entity, resource)
		{ }

		Signal<const ControllerButtonPressEvent&>	pressed;		///< Signal emitted when the controller button is pressed
		Signal<const ControllerButtonReleaseEvent&>	released;		///< Signal emitted when the controller button is released
		Signal<const ControllerAxisEvent&>			axisChanged;	///< Signal emitted when this component receives axis movement information

	protected:
		virtual void trigger(const nap::InputEvent& inEvent) override;
	};
}
