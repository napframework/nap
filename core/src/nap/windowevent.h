#pragma once

#include "event.h"

namespace nap
{
	/**
	 *	Base class for all window related events, only holds the window id
	 */
	class WindowEvent : public Event
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
	class ParameterizedWindowEvent : public WindowEvent
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
	class WindowShownEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowShownEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been hidden
	 */
	class WindowHiddenEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowHiddenEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been moved to the specified coordinates
	 */
	class WindowMovedEvent : public ParameterizedWindowEvent
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
	class WindowResizedEvent : public ParameterizedWindowEvent
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
	class WindowMinimizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowMinimizedEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has been maximized
	 */
	class WindowMaximizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowMaximizedEvent(int window) : WindowEvent(window) { }

	};

	/**
	 * Window has been restored to normal size and position
	 */
	class WindowRestoredEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowRestoredEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has gained mouse focus
	 */
	class WindowEnterEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowEnterEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has lost mouse focus
	 */
	class WindowLeaveEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowLeaveEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has gained keyboard focus
	 */
	class WindowFocusGainedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowFocusGainedEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * Window has lost keyboard focus
	 */
	class WindowFocusLostEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowFocusLostEvent(int window) : WindowEvent(window) { }
	};

	/**
	 * The window manager requests that the window be closed 
	 */
	class WindowCloseEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		WindowCloseEvent(int window) : WindowEvent(window) { }
	};

	using WindowEventPtr = std::unique_ptr<WindowEvent>;
	using WindowEventPtrList = std::vector<WindowEventPtr>;
}