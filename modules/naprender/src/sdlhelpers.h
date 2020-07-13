#pragma once

// External Includes
#include <string>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/numeric.h>

// SDL Forward declares
struct SDL_Window;

namespace nap
{
	namespace SDL
	{
		/**
		 * Makes a window visible.
		 * @param window pointer to the window to make visible.
		 * @param show if the window is shown or hidden
		 */
		void NAPAPI showWindow(SDL_Window* window, bool show);

		/**
		 * Makes sure the window is on top and receives input focus.
		 * @param window the window to raise
		 */
		void NAPAPI raiseWindow(SDL_Window* window);

		/**
		 * Sets the window to be full screen in desktop mode
		 * @param window the window to enable / disable
		 * @param value if the window is full screen
		 */
		void NAPAPI setFullscreen(SDL_Window* window, bool value);

		/**
		 * Returns if the window is full screen (Desktop)
		 */
		 bool NAPAPI getFullscreen(SDL_Window* window);

		/**
		 * Returns the size of an OpenGL window
		 * @param window the window to retrieve the size for
		 * @return the window size
		 */
		glm::ivec2 NAPAPI getWindowSize(SDL_Window* window);

		/**
		 * @param window the window to get the flags for
		 * @return flags associated with the given window
		 */
		uint32 NAPAPI getWindowFlags(SDL_Window* window);

		/**
		 * @param screenIndex the number of the display
		 * @return the screen resolution in pixels, on failure return value is -1
		 */
		glm::ivec2 NAPAPI getScreenSize(int screenIndex);

		/**
		 * resizes an OpenGL window
		 * @param window the window to resize
		 * @param size the new window size
		 */
		void NAPAPI setWindowSize(SDL_Window* window, const glm::ivec2& size);

		/**
		 * Returns the actual size in pixels of a window, which can be different from the represented window size
		 * This is the case with High DPI screens on OSX
		 * @return the actual size in pixels of a window
		 */
		glm::ivec2 NAPAPI getDrawableWindowSize(SDL_Window* window);

		/**
		 * returns the OpenGL window position as pixel coordinates
		 * @param window the window to get the position for
		 * @return the window position in pixels
		 */
		glm::ivec2 NAPAPI getWindowPosition(SDL_Window* window);

		/**
		 * Set the position of a window on screen.
		 * @param window the window to set the position for
		 * @param position the window location in pixels
		 */
		void NAPAPI setWindowPosition(SDL_Window* window, const glm::ivec2& position);

		/**
		 * Shutdown SDL
		 */
		void NAPAPI shutdownVideo();

		/**
		 * @ return the last SDL error as a string
		 */
		std::string NAPAPI getSDLError();

		/**
		 * @return the id associated with a specific opengl window
		 * @param window the opengl window to get the id for
		 */
		uint32_t NAPAPI getWindowId(SDL_Window* window);

		/**
		 * Hides the mouse cursor
		 */
		void NAPAPI hideCursor();

		/**
		 * Shows the mouse cursor
		 */
		void NAPAPI showCursor();

		/**
		 * Initializes SDL video system
		 * Call this before creating any windows or render contexts!
		 * @return if the system initialized correctly or not
		 */
		bool initVideo();

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
		 * Returns an SDL window based on the given ID
		 * @param id the window id to query
		 * @return handle to the SDL window, nullptr if not found
		 */
		SDL_Window* getWindow(uint32_t id);
	}
}