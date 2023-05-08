/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <utility/errorstate.h>

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
		 * Returns the size of an SDL window
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
		 * resizes an SDL window
		 * @param window the window to resize
		 * @param size the new window size
		 */
		void NAPAPI setWindowSize(SDL_Window* window, const glm::ivec2& size);

		/**
		 * Returns the actual size in pixels of a window, which can be different from the represented window size.
		 * This is the case with High DPI screens on OSX
		 * @return the actual size in pixels of a window
		 */
		glm::ivec2 NAPAPI getDrawableWindowSize(SDL_Window* window);

		/**
		 * returns the window position as pixel coordinates
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
		 * @return the id associated with a specific SDL window
		 * @param window the SDL window to get the id for
		 */
		uint32_t NAPAPI getWindowId(SDL_Window* window);

		/**
		 * Get the number of available video displays.
		 * @return A number >= 1, negative error code on failure;
		 */
		int NAPAPI getDisplayCount();

		/**
		 * Get the index of the display associated with a window.
		 * @param window the window to get the display index for 
		 * @return The index of the display containing the center of the window on success or a negative error code 
		 */
		int NAPAPI getDisplayIndex(SDL_Window* window);

		/**
		 * Get he dots/pixels-per-inch for a display.
		 * @param displayIndex The index of the display from which DPI information should be queried
		 * @param ddpi a pointer filled in with the diagonal DPI of the display; may be nullptr
		 * @param hdpi a pointer filled in with the horizontal DPI of the display; may be nullptr
		 * @param vdpi a pointer filled in with the vertical DPI of the display; may be nullptr
		 * @return 0 on success or a negative error code on failure
		 */
		int NAPAPI getDisplayDPI(int displayIndex, float* ddpi, float* hdpi, float* vdpi);

		/**
		 * Get the dots/pixels-per-inch of the display that holds the given window
		 * @param window the window to get the dpi for
		 * @param ddpi a pointer filled in with the diagonal DPI of the display; may be nullptr
		 * @param hdpi a pointer filled in with the horizontal DPI of the display; may be nullptr
		 * @param vdpi a pointer filled in with the vertical DPI of the display; may be nullptr
		 * @return 0 on success or a negative error code on failure
		 */
		int NAPAPI getDisplayDPI(SDL_Window* window, float* ddpi, float* hdpi, float* vdpi);

		/**
		 * @param displayIndex index of display to get name for  
		 * @return display name for given display index, nullptr on failure
		 */
		bool NAPAPI getDisplayName(int displayIndex, std::string& outName);

		/**
		 * Gets desktop area represented by a display, with the primary display located at 0,0
		 * @param displayIndex index of the display to get the bounds for
		 * @param outMin min position of desktop area represented by a display, with the primary display located at 0,0
		 * @param outMax max position of desktop area represented by a display, with the primary display located at 0,0
		 * @return 0 on success or a negative error code on failure
		 */
		int NAPAPI getDisplayBounds(int displayIndex, glm::ivec2& outMin, glm::ivec2& outMax);

		/**
		 * Hides the mouse cursor
		 */
		void NAPAPI hideCursor();

		/**
		 * Shows the mouse cursor
		 */
		void NAPAPI showCursor();

		/**
		 * Returns if the cursor is visible
		 * @return if the cursor is visible
		 */
		bool NAPAPI cursorVisible();

		/**
		 * Toggles cursor visibility on / off
		 */
		void NAPAPI toggleCursor();

		/**
		 * Returns the mouse cursor position relative to the focus window.
		 * @return cursor position relative to the focus window
		 */
		glm::ivec2 NAPAPI getCursorPosition();

		/**
		 * Get the position of the cursor, in relation to the desktop.
		 * This works just like getCursorPosition(), but the coordinates will be reported relative to the top-left of the desktop.
		 * Current position of the cursor, in relation to the desktop
		 */
		glm::ivec2 NAPAPI getGlobalCursorPosition();

		/**
		 * Get the current state of the mouse relative to the focus window.
		 * @param x the current x coordinate. Can be nullptr
		 * @param y the current y coordinate. Can be nullptr
		 * @return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
		 */
		uint32 NAPAPI getMouseState(int* x, int* y);

		/**
		 * Get the current state of the mouse, in relation to the desktop.
		 * This works just like getMouseState(), but the coordinates will be reported relative to the top-left of the desktop.
		 * @param x the current x coordinate, relative to the desktop. Can be nullptr
		 * @param y the current y coordinate, relative to the desktop. Can be nullptr
		 * @return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
		 */
		uint32 NAPAPI getGlobalMouseState(int* x, int* y);

		/**
		 * Initializes SDL video system.
		 * Call this before creating any windows or render contexts!
		 * @return if the system initialized correctly or not
		 */
		bool NAPAPI initVideo(utility::ErrorState& error);

		/**
	 	 * Controls if the window has any borders.
		 * @param window the window to set
		 * @param hasBorders if the window should have borders
		 */
		void NAPAPI setWindowBordered(SDL_Window* window, bool hasBorders);

		/**
		 * Sets the window title.
		 * @param window the window to set the title for
		 * @param name the new window name
		 */
		void NAPAPI setWindowTitle(SDL_Window* window, const std::string& name);
	}
}
