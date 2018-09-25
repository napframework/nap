#pragma once

// Local Includes
#include "keyboard.h"
#include "mouse.h"
#include "controller.h"

// External Includes
#include <rtti/rtti.h>
#include <nap/numeric.h>
#include <nap/event.h>

namespace nap
{
	/**
	 * Defines an input event that is passed along the system
	 */
	class NAPAPI InputEvent : public Event
	{
		RTTI_ENABLE(Event)
	};


	/**
	 * An input event associated with a specific window
	 * Most likely mouse, keyboard and touch events
	 */
	class NAPAPI WindowInputEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		WindowInputEvent(int window) : mWindow(window) { }
		int mWindow;					///< Window ID
	};


	//////////////////////////////////////////////////////////////////////////
	// Keyboard Input Events
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Keyboard input event
	 * 
	 * Contains all the information regarding keyboard interaction
	 */
	class NAPAPI KeyEvent : public WindowInputEvent
	{
		RTTI_ENABLE(WindowInputEvent)
	public:
		KeyEvent(EKeyCode inKey, int window = 0) : WindowInputEvent(window),
			mKey(inKey)	
		{
		}
		
		EKeyCode mKey;					///< Associated Key
	};


	/**
	 *	Key pressed event
	 */
	class NAPAPI KeyPressEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyPressEvent(EKeyCode inKey, int window = 0) : 
			KeyEvent(inKey, window) 
		{ 
		}
	};


	/**
	 *	Key released event
	 */
	class NAPAPI KeyReleaseEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyReleaseEvent(EKeyCode inKey, int window = 0) :
			KeyEvent(inKey, window) 
		{
		}
	};


	//////////////////////////////////////////////////////////////////////////
	// Mouse Input Events, always associated with a with a window
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Contains all relevant information for pointer specific interaction
	 * Can also be used to signal multi touch gestures (therefore the id)
	*/
	class NAPAPI PointerEvent : public WindowInputEvent
	{
		RTTI_ENABLE(WindowInputEvent)
	public:
		PointerEvent(int inX, int inY, int window = 0, int inId = 0) : WindowInputEvent(window),
			mX(inX), 
			mY(inY),
			mId(inId)		
		{ 
		}

		int		mX;							///< horizontal window pixel coordinate
		int		mY;							///< vertical window pixel coordinate
		int		mId;						///< device id
	};
	

	/**
	 *  Base class for all click related pointer events
	 */
	class NAPAPI PointerClickEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerClickEvent(int inX, int inY, EMouseButton inButton, int window = 0, int inId = 0) :
			PointerEvent(inX, inY, window, inId), 
			mButton(inButton)	
		{
		}

		EMouseButton mButton;				///< clicked mouse button
	};
	

	/**
	 *	Click occurred
	 */
	class NAPAPI PointerPressEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerPressEvent(int inX, int inY, EMouseButton inButton, int window=0, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, window, inId)
		{
		}
	};
	

	/**
	 *	Click has been released
	 */
	class NAPAPI PointerReleaseEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerReleaseEvent (int inX, int inY, EMouseButton inButton, int window=0, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, window, inId)
		{
		}
	};


	/**
	 *	Pointer movement occurred
	 */
	class NAPAPI PointerMoveEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerMoveEvent(int relX, int relY, int inAbsX, int inAbsY, int window=0, int inId = 0) : 
			PointerEvent(inAbsX, inAbsY, window, inId),
			mRelX(relX),
			mRelY(relY)
		{
		}

		int mRelX;							///< Horizontal relative movement in pixels
		int mRelY;							///< Vertical relative movement in pixels
	};


	/**
	 * Mouse wheel event
	 */
	class NAPAPI MouseWheelEvent : public WindowInputEvent
	{
		RTTI_ENABLE(WindowInputEvent)
	public:
		MouseWheelEvent(int x, int y, int window=0) : 
			WindowInputEvent(window),
			mX(x),
			mY(y)
		{
		}

		int mX;
		int mY;
	};


	//////////////////////////////////////////////////////////////////////////
	// Controller Input Events
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for all controller (game-pad / joystick) related events
	 */
	class NAPAPI ControllerEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		ControllerEvent(int deviceID) :
			mDeviceID(deviceID) { }

		int mDeviceID = -1;												///< Hardware device if of controller
	};

	/**
	 * Defines a controller axis event.
	 * This event is created when an axis on a game controller or joystick changes.
	 * Use the 'mAxis' variable to retrieve the mapped axis id, ie: LEFT_X or TRIGGER_RIGHT
	 * If there is no mapping available the 'mAxis' variable is unknown,
	 * in that case you need to interpret the hardware id stored in 'mAxisID' yourself
	 */
	class NAPAPI ControllerAxisEvent : public ControllerEvent
	{
		RTTI_ENABLE(ControllerEvent)
	public:
		ControllerAxisEvent(int deviceID, EControllerAxis axis, int axisID, double value) : 
			ControllerEvent(deviceID),
			mAxis(axis),
			mAxisID(axisID),
			mValue(value) { }

		EControllerAxis mAxis = EControllerAxis::UNKNOWN;				///< Mapped axis, unknown when no mapping is provided
		int mAxisID = -1;												///< Hardware axis id, available when no button mapping is provided (UNKNOWN)
		double mValue = 0.0;											///< Axis value ranging from -1.0, 1.0
	};

	/**
	 * Defines a controller button event
	 * This event is created when a button is pressed or released on a game controller or joystick.
	 * Use the 'mButton member to retrieve the mapped button id, ie: A, B, etc. 
	 * If there is no mapping available the 'mButton' member is unknown.
	 * In that case you need to interpret the hardware id of the button stored in the 'mButtonID' variable yourself.
	 */
	class NAPAPI ControllerButtonEvent : public ControllerEvent
	{
		RTTI_ENABLE(ControllerEvent)
	public:
		ControllerButtonEvent(int deviceID, EControllerButton button, int buttonID) :
			ControllerEvent(deviceID),
			mButton(button),
			mButtonID(buttonID) { }

		EControllerButton mButton = EControllerButton::UNKNOWN;			///< Mapped button, unknown when no mapping is provided
		int mButtonID = -1;												///< Hardware button id, available when no button mapping is provided (UNKNOWN)
	};


	/**
	 * Defines a controller button press event
	 * This event is created when a button on a joystick or controller is pressed
	 */
	class NAPAPI ControllerButtonPressEvent : public ControllerButtonEvent
	{
		RTTI_ENABLE(ControllerButtonEvent)
	public:
		ControllerButtonPressEvent(int deviceID, EControllerButton button, int buttonID) :
			ControllerButtonEvent(deviceID, button, buttonID) { }
	};


	/**
	 * Defines a controller button release event
	 * This event is created when a button on a joystick or controller is released
	 */
	class NAPAPI ControllerButtonReleaseEvent : public ControllerButtonEvent
	{
		RTTI_ENABLE(ControllerButtonEvent)
	public:
		ControllerButtonReleaseEvent(int deviceID, EControllerButton button, int buttonID) :
			ControllerButtonEvent(deviceID, button, buttonID) { }
	};


	/**
	 * Occurs when a controller is disconnected
	 */
	class NAPAPI ControllerConnectionEvent : public ControllerEvent
	{
		RTTI_ENABLE(ControllerEvent)
	public:
		ControllerConnectionEvent(int deviceID, bool status) : 
			ControllerEvent(deviceID),
			mStatus(status)				{ }
		bool mStatus = false;											///< If the controller connected (true) or disconnected (false)
	};


	using InputEventPtr = std::unique_ptr<nap::InputEvent>;
	using InputEventPtrList = std::vector<InputEventPtr>;
}
