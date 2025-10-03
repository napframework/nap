/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "videodriver.h"
#include "display.h"

// External Includes
#include <string>
#include <glm/glm.hpp>
#include <utility/dllexport.h>
#include <utility/errorstate.h>
#include <SDL_hints.h>
#include <SDL_surface.h>

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
		 * @return if full screen operation succeeded
		 */
		bool NAPAPI setFullscreen(SDL_Window* window, bool value);

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
		bool NAPAPI setWindowPosition(SDL_Window* window, const glm::ivec2& position);

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
		 * @return Number of unique displays, negative value on failure;
		 */
		int NAPAPI getDisplayCount();

		/**
		 * Returns list of unique display ids, 0 on failure 
		 * @return array of unique display ids, 0 on failure
		 */
		NAPAPI std::vector<int> getDisplayIDs();

		/**
		 * Returns a list of currently attached displays.
		 * @return a list of currently attached displays
		 */
		NAPAPI std::vector<nap::Display> getDisplays();

		/**
		 * Get the unique index of the display associated with a window.
		 * @param window the window to get the display index for
		 * @return The unique display index on success, negative on failure.
		 */
		int NAPAPI getDisplayIndex(SDL_Window* window);

		/**
		 * @param displayIndex index of display to get name for
		 * @param outName display name for given display index
		 */
		bool NAPAPI getDisplayName(int displayIndex, std::string& outName);

		/**
		 * Gets desktop area represented by a display, with the primary display located at 0,0
		 * @param displayIndex index of the display to get the bounds for
		 * @param outMin min position of desktop area represented by a display, with the primary display located at 0,0
		 * @param outMax max position of desktop area represented by a display, with the primary display located at 0,0
		 * @return true on success, false on failure
		 */
		bool NAPAPI getDisplayBounds(int displayIndex, glm::ivec2& outMin, glm::ivec2& outMax);

		/**
		 * Get the content scale of a display.
		 * 
		 * The content scale is the expected scale for content based on the DPI settings of the display.
		 * For example, a 4K display might have a 2.0 (200%) display scale,
		 * which means that the user expects UI elements to be twice as big on this display, to aid in readability.
		 *
		 * SDL_GetWindowDisplayScale() should be used to query the content scale factor for individual windows,
		 * instead of querying the display for a window and calling this function, as the per-window content scale factor may differ from the base value of the display it is on,
		 * particularly on high-DPI and/or multi-monitor desktop configurations.
		 * 
		 * @param displayIndex The index of the display
		 * @param scale the returned scale, 0.0 if call fails
		 * @return if the call succeeded
		 */
		bool NAPAPI getDisplayContentScale(int displayIndex, float* scale);

		/**
		 * Get the content scale of a display.
		 * 
		 * The content scale is the expected scale for content based on the DPI settings of the display.
		 * For example, a 4K display might have a 2.0 (200%) display scale,
		 * which means that the user expects UI elements to be twice as big on this display, to aid in readability.
		 *
		 * SDL_GetWindowDisplayScale() should be used to query the content scale factor for individual windows,
		 * instead of querying the display for a window and calling this function, as the per-window content scale factor may differ from the base value of the display it is on,
		 * particularly on high-DPI and/or multi-monitor desktop configurations.
		 * 
		 * @param window the window to get the content scale for
		 * @param scale the display content scaling factor, 0.0 if call fails
		 * @return if the call succeeded
		 */
		bool NAPAPI getDisplayContentScale(SDL_Window* window, float* scale);

		/**
		 *  @reutn the way a display is rotated.
		 */
		Display::EOrientation NAPAPI getDisplayOrientation(int displayIndex);

		/**
		 * Get the pixel density of a window, this is a ratio of pixel size to window size.
		 * 
		 * For example, if the window is 1920x1080 and it has a high density back buffer of 3840x2160 pixels,
		 * it would have a pixel density of 2.0.
		 *
		 * @param window the window to get the pixel density for
		 * @param density the window pixel density, 0.0 if call fails
		 * @return if call succeeded
		 */
		bool NAPAPI getWindowPixelDensity(SDL_Window* window, float* density);

		/**
		 * Get the content display scale relative to a window's pixel size.
		 * 
		 * This is a combination of the window pixel density and the display content scale, and is the expected scale for displaying content in this window.
		 * For example, if a 3840x2160 window had a display scale of 2.0, the user expects the content to take twice as many pixels and
		 * be the same physical size as if it were being displayed in a 1920x1080 window with a display scale of 1.0.
		 *
		 * Conceptually this value corresponds to the scale display setting, and is updated when that setting is changed,
		 * or the window moves to a display with a different scale setting.
		 *
		 * @param window the window to get the content scale for
		 * @param scale the window content scale, 0.0 if call fails
		 * @return if call succeeded
		 */
		bool NAPAPI getWindowDisplayScale(SDL_Window* window, float* scale);

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
		glm::vec2 NAPAPI getCursorPosition();

		/**
		 * Get the position of the cursor, in relation to the desktop.
		 * This works just like getCursorPosition(), but the coordinates will be reported relative to the top-left of the desktop.
		 * Current position of the cursor, in relation to the desktop
		 */
		glm::vec2 NAPAPI getGlobalCursorPosition();

		/**
		 * Get the current state of the mouse relative to the focus window.
		 * @param x the current x coordinate. Can be nullptr
		 * @param y the current y coordinate. Can be nullptr
		 * @return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
		 */
		uint32 NAPAPI getMouseState(float* x, float* y);

		/**
		 * Get the current state of the mouse, in relation to the desktop.
		 * This works just like getMouseState(), but the coordinates will be reported relative to the top-left of the desktop.
		 * @param x the current x coordinate, relative to the desktop. Can be nullptr
		 * @param y the current y coordinate, relative to the desktop. Can be nullptr
		 * @return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
		 */
		uint32 NAPAPI getGlobalMouseState(float* x, float* y);

		/**
		 * Get all available video drivers.
		 * @return all available video drivers
		 */
		NAPAPI std::vector<std::string> getVideoDrivers();

		/**
		 * Get the current SDL video driver.
		 * Note that the name of an SDL video driver is always lower-case, without capitals.
		 * @return current SDL video driver
		 */
		NAPAPI std::string getCurrentVideoDriver();

		/**
		 * Initializes SDL video system.
		 * Call this before creating any windows or render contexts!
		 * @return if the system initialized correctly or not
		 */
		bool NAPAPI initVideo(utility::ErrorState& error);

		/**
		 * Initializes the SDL video system using the given driver.
		 * Call this before creating any windows or render contexts!
		 * @param driver video back-end driver, call fails if driver is not available.
		 * @return if the system initialized using the selected driver.
		 */
		bool NAPAPI initVideo(EVideoDriver driver, utility::ErrorState& error);

		/**
		 * Returns if the video subsystem has been initialized 
		 * @return if the SDL video subsystem has been initialized
		 */
		bool NAPAPI videoInitialized();

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

		/**
		 * Brings the window to the front and keeps it there
		 * @param window the window to move to the front and keep on top
		 * @param enabled if always on top should be enabled
		 */
		void NAPAPI setWindowAlwaysOnTop(SDL_Window* window, bool enabled);

		/**
		 * Turn resizing of a window by a user on or off.
		 * @param window the resize window 
		 * @param enabled resize flag
		 */
		void NAPAPI setWindowResizable(SDL_Window* window, bool enabled);

		/**
		 * Create a 4 channel (RGBA) pixel surface from image on disk.
		 * RGB images are converted to RGBA, where the alpha channel is initialized to '255'. 
		 * @param imagePath absolute path to image on disk
		 * @param error the error if creation fails
		 * @return surface created from image, nullptr if creation failed
		 */
		using SurfacePtr = std::unique_ptr<SDL_Surface, std::function<void(SDL_Surface*)>>;
		NAPAPI SurfacePtr createSurface(const std::string& imagePath, utility::ErrorState& error);

		/**
		 * Start accepting Unicode text input events in a window.
		 * @param window the window to enable text input from
		 */
		bool NAPAPI enableTextInput(SDL_Window* window);

		/**
		 * Stop accepting Unicode text input events in a window.
		 * @param window the window to enable text input from
		 */
		bool NAPAPI disableTextInput(SDL_Window* window);
	}
}
