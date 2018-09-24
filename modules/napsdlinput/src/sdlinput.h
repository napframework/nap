#pragma once
#include "utility/dllexport.h"
#include <keyboard.h>
#include <SDL_events.h>
#include <inputevent.h>

namespace nap
{
	/**
	 * Utility function that checks if this is an input event (key, mouse or controller)
	 * @param sdlEvent the sdlEvent to verify
	 * @return if this sdl event is an input event
	 */
	bool NAPAPI isInputEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function to translate a SDL event to a generic nap InputEvent
	 * @param sdlEvent The event to translate
	 * @return Null if the sdlEvent is not an input event (or an unknown input event), the nap event otherwise
	 */
	nap::InputEventPtr NAPAPI translateInputEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function that checks if this is a key input event
	 * @param sdlEvent the sdl event to check
	 * @return if the SDL event is an input event
	 */
	bool NAPAPI isKeyEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function to translate an SDL event into a NAP key event
	 * This call assumes that the given SDL event can be translated into a NAP key event!
	 * Use isKeyEvent to verify if the events are compatible
	 * @param sdlEvent the sdl event to translate
	 * @return a nap key event
	 */
	nap::InputEventPtr NAPAPI translateKeyEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function that checks if the sdl event is a mouse input event
	 * @param sdlEvent the SDL event to verify
	 * @return if the event is a mouse input event
	 */
	bool NAPAPI isMouseEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function to translate an SDL event into a NAP mouse event
	 * This call assumes that the given SDL event can be translated into a NAP pointer (mouse) event!
	 * Use isMouseEvent to verify if the events are compatible
	 * @param sdlEvent the sdl mouse event to translate
	 * @return a nap pointer event
	 */
	nap::InputEventPtr NAPAPI translateMouseEvent(SDL_Event& sdlEvent);

	/**
	* Utility functions that checks if this is a controller input event (gamepad or joystick)
	* @param sdlEvent the sdlEvent to verify, both joystick and controller events are considered valid.
	* @return if this SDL event is a controller compatible event
	*/
	bool NAPAPI isControllerEvent(SDL_Event& sdlEvent);

	/**
	 * Utility function to translate an SDL event into a NAP controller event
	 * This call assumes that the given SDL event can be translated into a NAP controller event!
	 * Use isControllerEvent to verify if the events are compatible
	 * @param sdlEvent the SDL event to translate, can be from a joystick or controller
 	 * @return a nap controller event
	 */
	nap::InputEventPtr NAPAPI translateControllerEvent(SDL_Event& sdlEvent);
}