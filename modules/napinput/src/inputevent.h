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
	 * Base class for all controller related events
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
	 * This event can be constructed using a pre-defined axis (when mapping is known),
	 * or a generic axis when no mapping is available. 
	 * In that case the axis member is set to be UNKNOWN
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
		int mAxisID = 0;												///< Hardware id associated with the incoming axis, always available
		double mValue = 0.0;											///< Axis value ranging from -1.0, 1.0
	};

	/**
	 * Defines a controller button event
	 * This event can be constructed using a pre-defined  button (when mapping is known), 
	 * or a generic button id when no mapping is available.
	 * In that case the button member is set to be UNKNOWN
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
		int mButtonID = -1;												///< Hardware button id, always available
	};

	/**
	 *	Defines a controller button press event
	 */
	class NAPAPI ControllerButtonPressEvent : public ControllerButtonEvent
	{
		RTTI_ENABLE(ControllerButtonEvent)
	public:
		ControllerButtonPressEvent(int deviceID, EControllerButton button, int buttonID) :
			ControllerButtonEvent(deviceID, button, buttonID) { }
	};

	/**
	 *	Defines a controller button release event
	 */
	class NAPAPI ControllerButtonReleaseEvent : public ControllerButtonEvent
	{
		RTTI_ENABLE(ControllerButtonEvent)
	public:
		ControllerButtonReleaseEvent(int deviceID, EControllerButton button, int buttonID) :
			ControllerButtonEvent(deviceID, button, buttonID) { }
	};

	using InputEventPtr = std::unique_ptr<nap::InputEvent>;
	using InputEventPtrList = std::vector<InputEventPtr>;
}
