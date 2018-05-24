#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/numeric.h>
#include <nap/event.h>
#include "keycode.h"
#include "mousebutton.h"

namespace nap
{
	/**
	 * Defines an input event that is passed along the system
	 */
	class NAPAPI InputEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		InputEvent(int window) : mWindow(window) { }
		int mWindow;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Keyboard input event
	 * 
	 * Contains all the information regarding keyboard interaction
	 */
	class NAPAPI KeyEvent : public InputEvent
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

	/**
	 * Contains all relevant information for pointer specific interaction
	 * Can also be used to signal multi touch gestures (therefore the id)
	*/
	class NAPAPI PointerEvent : public InputEvent
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

		EMouseButton mButton;
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

		int mRelX;
		int mRelY;
	};

	/**
	 * Mouse wheel event
	 */
	class NAPAPI MouseWheelEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		MouseWheelEvent(int x, int y, int window=0) : 
			InputEvent(window),
			mX(x),
			mY(y)
		{
		}

		int mX;
		int mY;
	};

	using InputEventPtr = std::unique_ptr<nap::InputEvent>;
	using InputEventPtrList = std::vector<InputEventPtr>;
}
