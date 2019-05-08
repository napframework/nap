#pragma once

// External Includes
#include <utility/dllexport.h>
#include <nap/event.h>

namespace nap
{
	/**
	 *	Base class for all window related events, only holds the window id
	 */
	class NAPAPI WindowEvent : public Event
	{
	public:
		RTTI_ENABLE(Event)

	public:
		WindowEvent(int window) : mWindow(window) 
		{}
		
		int mWindow;
	};

	/**
	 *	Base class for all window events that have parameters associated with it
	 */
	class NAPAPI ParameterizedWindowEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		ParameterizedWindowEvent(int inX, int inY, int window) : WindowEvent(window),
			mX(inX), mY(inY)		
		{
		}

		int mX;
		int mY;
	};

	//////////////////////////////////////////////////////////////////////////

	/** 
	 *	Window has been shown 
	 */
	class NAPAPI WindowShownEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowShownEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been hidden
	 */
	class NAPAPI WindowHiddenEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowHiddenEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been moved to the specified coordinates
	 */
	class NAPAPI WindowMovedEvent : public ParameterizedWindowEvent
	{
		RTTI_ENABLE(ParameterizedWindowEvent)
	public:
		WindowMovedEvent(int x, int y, int window) : ParameterizedWindowEvent(x, y, window)
		{
		}
	};

	/**
	 * Window has been resized to the specified size
	 */
	class NAPAPI WindowResizedEvent : public ParameterizedWindowEvent
	{
		RTTI_ENABLE(ParameterizedWindowEvent)
	public:
		WindowResizedEvent(int width, int height, int window) :ParameterizedWindowEvent(width, height, window) 
		{
		}
	};

	/**
	 * Window has been minimized
	 */
	class NAPAPI WindowMinimizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowMinimizedEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been maximized
	 */
	class NAPAPI WindowMaximizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowMaximizedEvent(int window) : WindowEvent(window) { }

	};

	/**
	 * Window has been restored to normal size and position
	 */
	class NAPAPI WindowRestoredEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowRestoredEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has gained mouse focus
	 */
	class NAPAPI WindowEnterEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowEnterEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has lost mouse focus
	 */
	class NAPAPI WindowLeaveEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowLeaveEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has gained keyboard focus
	 */
	class NAPAPI WindowFocusGainedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowFocusGainedEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has lost keyboard focus
	 */
	class NAPAPI WindowFocusLostEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowFocusLostEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * The window manager requests that the window be closed 
	 */
	class NAPAPI WindowCloseEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowCloseEvent(int window) : WindowEvent(window) { }
	};

	/**
	 *	Window has been exposed
	 */
	class NAPAPI WindowExposedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowExposedEvent(int window) : WindowEvent(window) { }
	};

	/**
	 *	Window is being offered focus
	 */
	class NAPAPI WindowTakeFocusEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowTakeFocusEvent(int window) : WindowEvent(window) { }
	};

	using WindowEventPtr = std::unique_ptr<WindowEvent>;
	using WindowEventPtrList = std::vector<WindowEventPtr>;
}