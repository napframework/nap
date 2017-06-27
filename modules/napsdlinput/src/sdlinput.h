#pragma once
#include "KeyCode.h"
#include "SDL_events.h"
#include "nap/event.h"

namespace nap
{
	/**
	 * Utility function to translate a SDL event to a generic nap InputEvent
	 *
	 * @param sdlEvent The event to translate
	 * @param windowID The window ID that the event is for (output)
	 * @return Null if the sdlEvent is not an input event (or an unknown input event), the nap event otherwise
	 */
	nap::EventPtr translateInputEvent(SDL_Event& sdlEvent, uint32_t& windowID);
}