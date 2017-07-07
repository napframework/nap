#pragma once

#include "event.h"

namespace nap
{
	/**
	 *	Base class for all window related events
	 */
	class WindowEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 *	Base class for all window events that have parameters associated with it
	 */
	class ParameterizedWindowEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	public:
		ParameterizedWindowEvent(int inX, int inY) : 
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
	};

	/**
	 * Window has been hidden
	 */
	class WindowHiddenEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has been moved to the specified coordinates
	 */
	class WindowMovedEvent : public ParameterizedWindowEvent
	{
		RTTI_ENABLE(ParameterizedWindowEvent)
	public:
		WindowMovedEvent(int x, int y) : ParameterizedWindowEvent(x, y)
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
		WindowResizedEvent(int width, int height) :ParameterizedWindowEvent(width, height) 
		{
		}
	};

	/**
	 * Window has been minimized
	 */
	class WindowMinimizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has been maximized
	 */
	class WindowMaximizedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has been restored to normal size and position
	 */
	class WindowRestoredEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has gained mouse focus
	 */
	class WindowEnterEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has lost mouse focus
	 */
	class WindowLeaveEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has gained keyboard focus
	 */
	class WindowFocusGainedEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * Window has lost keyboard focus
	 */
	class WindowFocusLostEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};

	/**
	 * The window manager requests that the window be closed 
	 */
	class WindowCloseEvent : public WindowEvent
	{
		RTTI_ENABLE(WindowEvent)
	};
}