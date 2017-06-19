#pragma once

// External Includes
#include <SDL.h>
#include <string>
#include <stdint.h>
#include <memory>

// Local Includes
#include "nwindow.h"

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}
}

namespace opengl
{
	/**
	 * Attributes
	 * 
	 * Set of default opengl attributes that should be initialized before creating a context
	 */ 
	struct Attributes
	{
		Attributes()  = default;
		~Attributes() = default;

		int  versionMajor = 3;			// Major GL Version
		int  versionMinor = 2;			// Minor GL Version
		bool doubleBuffer = true;		// Enables / Disabled double buffering
		bool debug = false;				// Whether to use the debug version of the OpenGL driver. Provides more debugging output.

		// TODO: FIGURE OUT WHY THERE DON'T SEEM TO HAVE AN EFFECT ON WINDOWS
		bool enableMultiSampling = 1;	// Enables / Disables multi sampling.
		int  multiSampleSamples = 4;	// Number of samples per pixel when multi sampling is enabled
	};


	/**
	 * setAttributes
	 * 
	 * sets the opengl attributes for the OpenGL context to create
	 * important to call this function after calling initVideo
	 * @param attributes: the attributes to set
	 */
	void setAttributes(const Attributes& attributes);


	/**
	* createWindow
	*
	* Creates a window with an associated OpenGL context
	* Note that it's possible to have one window share multiple contexts
	* But every context needs to be associated with at least 1 window
	*/
	std::unique_ptr<Window> createWindow(const WindowSettings& settings, nap::utility::ErrorState& errorState);


	/**
	 * hides the window
	 * @param window the window to hide
	 */
	void hideWindow(Window& window);


	/**
	 * shows the window
	 * @param window the window to show
	 */
	void showWindow(Window& window);


	/**
	 * sets the window position
	 * @param posx, posy: pixel coordinates on screen
	 */
	void setWindowPosition(Window& window, int x, int y);


	/**
	 * returns the window position 
	 * @param posx, posy: new position coordinates
	 */
	void getWindowPosition(Window& window, int &x, int &y);


	/**
	 * updates the window title
	 * @param title: the new window title
	 */
	void setWindowTitle(Window& window, const std::string& title);


	/**
	 * updates the window size
	 * @param size: the new window size
	 */
	void setWindowSize(Window& window, int width, int height);


	/**
	 * returns current window size
	 * @param window: the window to get current size for
	 * @param width: width of window
	 * @param height: height of window
	 */
	void getWindowSize(Window& window, int& width, int& height);


	/**
	 * Turn window v-sync on / off
	 */
	void setVSync(Window& window, bool value);


	/**
	* swap
	*
	* Swaps buffer for the window with what was drawn in the currently active context
	* Only possible when double buffering is enabled
	* @param window: the window to swap buffers for
	*/
	void swap(Window& window);


	/**
	 * Makes this opengl window's context current
	 * @param window the window's context to make current
	 */
	void makeCurrent(Window& window);


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

}	// opengl
