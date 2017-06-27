#pragma once
#include "SDL_events.h"
#include "nap/event.h"

namespace nap
{
	/**
	 * Utility function to translate an SDL event to a generic nap window event
	 *
	 * @param sdlEvent The event to translate
	 * @param windowID The window ID that the event is for (output)
	 * @return Null if the sdlEvent is not a window event (or an unknown window event), the nap event otherwise
	 */
	nap::EventPtr translateWindowEvent(SDL_Event& sdlEvent, uint32_t& windowID);
}