#pragma once
#include "SDL_events.h"
#include "nap/event.h"

namespace nap
{
	nap::EventPtr translateWindowEvent(SDL_Event& sdlEvent, uint32_t& windowID);
}