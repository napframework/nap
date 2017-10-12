#pragma once
#include "utility/dllexport.h"
#include "SDL_events.h"
#include <nap/event.h>
#include <nap/windowevent.h>

namespace nap
{
	/**
	 * Utility function to translate an SDL event to a generic nap window event
	 *
	 * @param sdlEvent The event to translate
	 * @param windowID The window ID that the event is for (output)
	 * @return Null if the sdlEvent is not a window event or if the event is for a window that has already been destroyed. The nap event otherwise.
	 */
	nap::WindowEventPtr NAPAPI translateWindowEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function to check if the sdl event is a window event
	 * @param sdlEvent the sdl event to verify
	 * @return if the sdl event is a window event
	 */
	bool NAPAPI isWindowEvent(SDL_Event& sdlEvent);
}