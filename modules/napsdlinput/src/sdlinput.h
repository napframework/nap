#pragma once
#include "KeyCode.h"
#include "SDL_events.h"
#include "nap/event.h"

namespace nap
{
	nap::EventPtr translateInputEvent(SDL_Event& sdlEvent, uint32_t& windowID);
}