#include "sdlwindow.h"
#include <nap/windowevent.h>
#include <nap/logger.h>
#include <assert.h>

namespace nap
{
	// Binds a specific sdl mouse event to a pointer event type
	static std::unordered_map<Uint32, rtti::TypeInfo> SDLToWindowMapping =
	{
		std::make_pair(SDL_WINDOWEVENT_SHOWN,			RTTI_OF(nap::WindowShownEvent)),
		std::make_pair(SDL_WINDOWEVENT_HIDDEN,			RTTI_OF(nap::WindowHiddenEvent)),
		std::make_pair(SDL_WINDOWEVENT_MINIMIZED,		RTTI_OF(nap::WindowMinimizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_MAXIMIZED,		RTTI_OF(nap::WindowMaximizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_RESTORED,		RTTI_OF(nap::WindowRestoredEvent)),
		std::make_pair(SDL_WINDOWEVENT_ENTER,			RTTI_OF(nap::WindowEnterEvent)),
		std::make_pair(SDL_WINDOWEVENT_LEAVE,			RTTI_OF(nap::WindowLeaveEvent)),
		std::make_pair(SDL_WINDOWEVENT_FOCUS_GAINED,	RTTI_OF(nap::WindowFocusGainedEvent)),
		std::make_pair(SDL_WINDOWEVENT_FOCUS_LOST,		RTTI_OF(nap::WindowFocusLostEvent)),
		std::make_pair(SDL_WINDOWEVENT_CLOSE,			RTTI_OF(nap::WindowCloseEvent)),
		std::make_pair(SDL_WINDOWEVENT_RESIZED,			RTTI_OF(nap::WindowResizedEvent)),
		std::make_pair(SDL_WINDOWEVENT_MOVED,			RTTI_OF(nap::WindowMovedEvent))
	};


	nap::WindowEventPtr translateWindowEvent(SDL_Event& sdlEvent)
	{
		// Get the binding and create correct event
		// If the event can't be located there's no valid event mapping 
		auto window_it = SDLToWindowMapping.find(sdlEvent.window.event);
		if (window_it == SDLToWindowMapping.end())
			return nullptr;

		int window_id = static_cast<int>(sdlEvent.window.windowID);

		// If it's one of the two parameterized constructors, add the arguments
		rtti::TypeInfo event_type = window_it->second;
		if (event_type.is_derived_from(RTTI_OF(nap::ParameterizedWindowEvent)))
		{
			return WindowEventPtr(event_type.create<WindowEvent>({ sdlEvent.window.data1, sdlEvent.window.data2, window_id }));
		}

		// Create and return correct window event
		return WindowEventPtr(event_type.create<nap::WindowEvent>({ window_id }));
	}


	bool isWindowEvent(SDL_Event& sdlEvent)
	{
		return SDLToWindowMapping.find(sdlEvent.window.event) != SDLToWindowMapping.end();
	}

}