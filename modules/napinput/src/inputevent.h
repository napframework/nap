#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/configure.h>
#include <nap/event.h>
#include "keycode.h"
#include "mousebutton.h"

namespace nap
{
	/**
	@brief InputEvent

	Defines an input event that is passed along the system
	**/
	class InputEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		InputEvent(int window) : mWindow(window)
		{
		}

		int mWindow;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief KeyEvent

	Key input event, tied to pressed and released
	**/
	class KeyEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		KeyEvent(EKeyCode inKey, int window = 0) : InputEvent(window),
			mKey(inKey)	
		{
		}
		
		EKeyCode mKey;
	};

	/**
	 *	Key has been pressed
	 */
	class KeyPressEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyPressEvent(EKeyCode inKey, int window = 0) : 
			KeyEvent(inKey, window) 
		{ 
		}
	};

	/**
	 *	Key has been released
	 */
	class KeyReleaseEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyReleaseEvent(EKeyCode inKey, int window = 0) :
			KeyEvent(inKey, window) 
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Pointer Events

	Contains all relevant information for pointer specific interaction
	Can also be used to signal multi touch gestures (therefore the id)
	**/
	class PointerEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		PointerEvent(int inX, int inY, int window = 0, int inId = 0) : InputEvent(window),
			mX(inX), 
			mY(inY),
			mId(inId)		
		{ 
		}

		int		mX;
		int		mY;
		int		mId;
	};
	
	/**
	 *	@brief PointerClickEvent
	 *  Base class for all click related pointer events
	 */
	class PointerClickEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerClickEvent(int inX, int inY, EMouseButton inButton, int window = 0, int inId = 0) :
			PointerEvent(inX, inY, window, inId), 
			mButton(inButton)	
		{
		}

		EMouseButton mButton;
	};
	
	/**
	 *	Click occurred
	 */
	class PointerPressEvent : public PointerClickEvent
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
	class PointerReleaseEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerReleaseEvent (int inX, int inY, EMouseButton inButton, int window=0, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, window, inId)
		{
		}
	};

	/**
	 *	Pointer drag occurred
	 */
	class PointerDragEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerDragEvent (int inX, int inY, EMouseButton inButton, int window=0, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, window, inId)
		{
		}
	};

	/**
	 *	Pointer movement occurred
	 */
	class PointerMoveEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerMoveEvent(int inX, int inY, int window=0, int inId = 0) : 
			PointerEvent(inX, inY, window, inId)
		{
		}
	};

	using InputEventPtr = std::unique_ptr<nap::InputEvent>;
	using InputEventPtrList = std::vector<InputEventPtr>;
}
