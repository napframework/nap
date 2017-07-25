#pragma once
#include <keycode.h>
#include <SDL_events.h>
#include <inputevent.h>

namespace nap
{
	/**
	 * Utility function to translate a SDL event to a generic nap InputEvent
	 *
	 * @param sdlEvent The event to translate
	 * @return Null if the sdlEvent is not an input event (or an unknown input event), the nap event otherwise
	 */
	nap::InputEventPtr translateInputEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function that checks if this is a key input event
	 * @param sdlEvent the sdl event to check
	 * @return if the SDL event is an input event
	 */
	bool isKeyEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function that checks if the sdl event is a mouse input event
	 * @param sdlEvent the SDL event to verify
	 * @return if the event is a mouse input event
	 */
	bool isPointerEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function that checks if this is an input event (key, mouse)
	 * @param sdlEvent the sdlEvent to verify
	 * @return if this sdl event is an input event
	 */
	bool isInputEvent(SDL_Event& sdlEvent);
}