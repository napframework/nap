#include "sdlwindow.h"
#include "nap/windowevent.h"

namespace nap
{
	nap::EventPtr translateWindowEvent(SDL_Event& sdlEvent, uint32_t& windowID)
	{
		// We're only interested in window events
		if (sdlEvent.type != SDL_WINDOWEVENT)
			return nullptr;
		
		windowID = sdlEvent.window.windowID;
		switch (sdlEvent.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
			return std::make_unique<nap::WindowShownEvent>();

		case SDL_WINDOWEVENT_HIDDEN:
			return std::make_unique<nap::WindowHiddenEvent>();

		case SDL_WINDOWEVENT_MOVED:
			return std::make_unique<nap::WindowMovedEvent>(sdlEvent.window.data1, sdlEvent.window.data2);

		case SDL_WINDOWEVENT_RESIZED:
			return std::make_unique<nap::WindowResizedEvent>(sdlEvent.window.data1, sdlEvent.window.data2);

		case SDL_WINDOWEVENT_MINIMIZED:
			return std::make_unique<nap::WindowMinimizedEvent>();

		case SDL_WINDOWEVENT_MAXIMIZED:
			return std::make_unique<nap::WindowMaximizedEvent>();

		case SDL_WINDOWEVENT_RESTORED:
			return std::make_unique<nap::WindowRestoredEvent>();

		case SDL_WINDOWEVENT_ENTER:
			return std::make_unique<nap::WindowEnterEvent>();

		case SDL_WINDOWEVENT_LEAVE:
			return std::make_unique<nap::WindowLeaveEvent>();

		case SDL_WINDOWEVENT_FOCUS_GAINED:
			return std::make_unique<nap::WindowFocusGainedEvent>();

		case SDL_WINDOWEVENT_FOCUS_LOST:
			return std::make_unique<nap::WindowFocusLostEvent>();

		case SDL_WINDOWEVENT_CLOSE:
			return std::make_unique<nap::WindowCloseEvent>();
		}
		
		return nullptr;
	}
}