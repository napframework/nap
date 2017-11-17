#pragma once

// External Includes
#include <string>
#include <SDL.h>
#include <glm/glm.hpp>

namespace opengl
{
	/**
	 *	Typedef for SDL_Event
	 */
	using Event = SDL_Event;

	/**
	 * Turn window v-sync on / off
	 */
	void enableVSync(bool value);

	/**
	 * Initializes SDL video system
	 * Call this before creating any windows or render contexts!
	 * @return if the system initialized correctly or not
	 */
	bool initVideo();

	/**
	 * Initializes the OpenGL GLEW wrapper
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
	 * @return the current opengl context
	 */
	SDL_GLContext getCurrentContext();

	/**
	 * Swap the OpenGL buffers for a window, if double-buffering is supported.
	 * @param the window to swap the buffers for
	 */
	void swap(SDL_Window* window);

	/**
	 * Show or show a window
	 * @param show if the window is shown or hidden
	 */
	void showWindow(SDL_Window* window, bool show);

	/**
	 * Sets the window to be full screen in desktop mode
	 * @param window the window to enable / disable
	 * @param value if the window is full screen 
	 */
	void setFullscreen(SDL_Window* window, bool value);

	/**
	 * Set up an OpenGL context for rendering into an OpenGL window.
	 * The context must have been created with a compatible window.
	 * @param window the window associated with the context
	 * @param context the context to set up for rendering
	 */
	void makeCurrent(SDL_Window* window, SDL_GLContext context);

	/**
	 * Set an OpenGL window attribute before window creation.
	 * @param window attribute to set
	 * @param value the value to set the window attribute to
	 */
	void setAttribute(SDL_GLattr attribute, int value);

	/**
	 * @return The value of a window attribute associated with the currently active context.
	 */
	int getAttribute(SDL_GLattr attribute);

	/**
	 * Creates an opengl context associated with @window
	 * @param window the window associated with the opengl context
	 * @return the new OpenGL context
	 */
	SDL_GLContext createContext(SDL_Window* window);

	/**
	 * Returns the size of an OpenGL window
	 * @param window the window to retrieve the size for
	 * @return the window size
	 */
	glm::ivec2 getWindowSize(SDL_Window* window);

	/**
	 * resizes an OpenGL window
	 * @param window the window to resize
	 * @param size the new window size
	 */
	void setWindowSize(SDL_Window* window, const glm::ivec2& size);

	/**
	 * returns the OpenGL window position as pixel coordinates
	 * @param window the window to get the position for
	 * @return the window position in pixels
	 */
	glm::ivec2 getWindowPosition(SDL_Window* window);

	/**
	 * Set the window position
	 * @param window the window to set the position for
	 * @param location the window location in pixels
	 */
	void setWindowPosition(SDL_Window* window, const glm::ivec2& position);

	/**
	 * Shutdown SDL
	 */
	void shutdown();

	/**
	 * @ return the last SDL error as a string
	 */
	std::string getSDLError();

	/**
	 * Prints the last SDL error to screen
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
