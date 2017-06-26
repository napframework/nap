#pragma once

#include "event.h"

namespace nap
{
	/** 
	 *	Window has been shown 
	 */
	class WindowShownEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has been hidden
	 */
	class WindowHiddenEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has been moved to the specified coordinates
	 */
	class WindowMovedEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		WindowMovedEvent() = default;
		WindowMovedEvent(int x, int y) : 
			mX(x),
			mY(y)
		{
		}

		int mX;
		int mY;
	};

	/**
	 * Window has been resized to the specified size
	 */
	class WindowResizedEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		WindowResizedEvent() = default;
		WindowResizedEvent(int width, int height) : 
			mWidth(width),
			mHeight(height)
		{
		}

		int mWidth;
		int mHeight;
	};

	/**
	 * Window has been minimized
	 */
	class WindowMinimizedEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has been maximized
	 */
	class WindowMaximizedEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has been restored to normal size and position
	 */
	class WindowRestoredEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has gained mouse focus
	 */
	class WindowEnterEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has lost mouse focus
	 */
	class WindowLeaveEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has gained keyboard focus
	 */
	class WindowFocusGainedEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * Window has lost keyboard focus
	 */
	class WindowFocusLostEvent : public Event
	{
		RTTI_ENABLE(Event)
	};

	/**
	 * The window manager requests that the window be closed 
	 */
	class WindowCloseEvent : public Event
	{
		RTTI_ENABLE(Event)
	};
}