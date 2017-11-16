#pragma once

// External Includes
#include <string>
#include <SDL.h>

namespace opengl
{
	/**
	 *	Typedef for SDL_Event
	 */
	using Event = SDL_Event;

	/**
	 * Turn window v-sync on / off
	 */
	void setVSync(bool value);

	/**
	 * initVideo
	 *
	 * Initializes SDL video system
	 * Call this before creating any windows or render contexts!
	 * TODO: Move to sdl header
	 * @return: if the system initialized correctly or not
	 */
	bool initVideo();

	/**
	 * initOpenGL
	 *
	 * Initializes the OpenGL glew wrapper
	 * This call will only succeed when OpenGL has a valid current context
	 */
	bool init();

	/**
	 * Deletes an SDL context
	 * @param context the context to delete
	 */
	void deleteContext(SDL_GLContext context);

	/**
	 * sDeletes an SDL window
	 * @param window the window to delete
	 */
	void deleteWindow(SDL_Window* window);

	/**
	 * Sets if the window is resizable or not
	 * @param window the window that should be resizable
	 * @param resize if the window is resizable
	 */
	void setWindowResizable(SDL_Window* window, bool resizable);

	/**
	 * Controls if the window has any borders
	 * @param window the window to set
	 * @param hasBorders if the window should have borders
	 */
	void setWindowBordered(SDL_Window* window, bool hasBorders);

	/**
	 * Sets the window title
	 * @param window the window to set the title for
	 * @param name the new window name
	 */
	void setWindowTitle(SDL_Window* window, const std::string& name);

	/**
	 * quit
	 *
	 * Shutdown SDL
	 * TODO: Should go in to a separate sdl header in a different project
	 */
	void shutdown();

	/**
	 * getSDLError
	 *
	 * Returns the last SDL error as a string
	 * TODO: Should go in to a separate sdl header in a different project 
	 */
	std::string getSDLError();


	/**
	 * printSDLError
	 *
	 * Prints the last SDL error to screen
	 * TODO: Should go in to a separate sdl header in a different project 
	 */
	void printSDLError();


	/**
	 * Polls SDL for an event
	 * @param event the event that was generated
	 * @return if an event has been generated
	 */
	bool pollEvent(opengl::Event& inputEvent);


	/**
	 * Returns an SDL window based on the given ID
	 * @param id, the window id to query
	 * @return handle to the SDL window, nullptr if not found
	 */
	SDL_Window* getWindow(uint32_t id);

	/**
	 * @return the id associated with a specific opengl window
	 * @param window, the opengl window to get the id for
	 */
	uint32_t getWindowId(SDL_Window* window);

}	// opengl
