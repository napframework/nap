#pragma once

// Local Includes
#include "sdlinputservice.h"

// External Includes
#include <utility/dllexport.h>
#include <keyboard.h>
#include <SDL_events.h>
#include <inputevent.h>
#include <windowevent.h>

namespace nap
{
	/**
	 * Converts SDL events into NAP Input Events
	 */
	class NAPAPI SDLEventConverter final
	{
	public:
		/**
		 * Constructor requires service to map device information
		 */
		SDLEventConverter(SDLInputService& service) : mService(service) { }

		/**
		 *	Default destructor
		 */
		virtual ~SDLEventConverter() = default;

		/**
		 * Copy is not allowed
		 */
		SDLEventConverter(SDLEventConverter&) = delete;
		SDLEventConverter& operator=(const SDLEventConverter&) = delete;

		/**
		 * Move is not allowed
		 */
		SDLEventConverter(SDLEventConverter&&) = delete;
		SDLEventConverter& operator=(SDLEventConverter&&) = delete;

		/**
		* Utility function that checks if this is an input event (key, mouse or controller)
		* @param sdlEvent the sdlEvent to verify
		* @return if this sdl event is an input event
		*/
		bool isInputEvent(SDL_Event& sdlEvent) const;

		/**
		* Utility function to translate a SDL event to a generic nap InputEvent
		* @param sdlEvent The event to translate
		* @return Null if the sdlEvent is not an input event (or an unknown input event), the nap event otherwise
		*/
		nap::InputEventPtr translateInputEvent(SDL_Event& sdlEvent);

		/**
		* Utility function that checks if this is a key input event (key press down/up or text input)
		* @param sdlEvent the sdl event to check
		* @return if the SDL event is an input event
		*/
		bool isKeyEvent(SDL_Event& sdlEvent) const;

		/**
		* Utility function to translate an SDL event into a NAP key event
		* This call assumes that the given SDL event can be translated into a NAP key event!
		* Use isKeyEvent() to verify if the events are compatible
		* @param sdlEvent the sdl event to translate
		* @return a nap key event, nullptr if the event could not be translated
		*/
		nap::InputEventPtr translateKeyEvent(SDL_Event& sdlEvent);

		/**
		* Utility function that checks if the sdl event is a mouse input event
		* @param sdlEvent the SDL event to verify
		* @return if the event is a mouse input event
		*/
		bool isMouseEvent(SDL_Event& sdlEvent) const;

		/**
		* Utility function to translate an SDL event into a NAP mouse event.
		* This call assumes that the given SDL event can be translated into a NAP pointer (mouse) event!
		* Use isMouseEvent() to verify if the events are compatible.
		* @param sdlEvent the sdl mouse event to translate
		* @return a nap pointer event, nullptr if the event could not be translated
		*/
		nap::InputEventPtr translateMouseEvent(SDL_Event& sdlEvent);

		/**
		* Utility functions that checks if this is a controller input event (gamepad or joystick).
		* @param sdlEvent the sdlEvent to verify, both joystick and controller events are considered valid.
		* @return if this SDL event is a controller compatible event
		*/
		bool isControllerEvent(SDL_Event& sdlEvent) const;

		/**
		* Utility function to convert an SDL event into a nap controller event.
		* This call assumes that the given SDL event can be translated into a NAP controller event!
		* SDL Joystick and SDL Controller events are considered to be valid.
		* Use isControllerEvent() to verify if the events are compatible.
		* @param sdlEvent the SDL event to translate, can be from a joystick or controller
		* @return a nap controller event, nullptr if event could not be translated
		*/
		nap::InputEventPtr translateControllerEvent(SDL_Event& sdlEvent);

		/**
		 * Utility function to check if the sdl event is a nap window event.
	 	 * @param sdlEvent the sdl event to verify
		 * @return if the sdl event is a window event
		 */
		bool isWindowEvent(SDL_Event& sdlEvent) const;

		/**
		 * Utility function to convert a generic SDL event into a generic nap window event
		 * @param sdlEvent The event to convert to a nap window event.
		 * @return Null if the sdlEvent is not a window event or if the event is for a window that has already been destroyed. The nap event otherwise.
		 */
		nap::WindowEventPtr translateWindowEvent(SDL_Event& sdlEvent);

	private:
		SDLInputService& mService;

		/**
		 * Utility function that translates an SDL joystick or controller event into a nap input event.
		 * @param sdlEvent the SDL event to translate
		 * @param sdlType the type of the SDL event
		 * @param eventType the nap input event to create based on the given sdlType
		 */ 
		nap::InputEvent* translateSDLControllerEvent(SDL_Event& sdlEvent, uint32 sdlType, const rtti::TypeInfo& eventType);
	};
}
