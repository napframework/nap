#pragma once

// External Includes
#include <SDL.h>
#include <string>
#include <stdint.h>

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
		bool dubbleBuffer = true;		// Enables / Disabled double buffering
		
		// TODO: FIGURE OUT WHY THERE DON'T SEEM TO HAVE AN EFFECT ON WINDOWS
		bool enableMultiSampling = 1;	// Enables / Disables multi sampling.
		int  multiSampleSamples = 4;	// Number of samples per pixel when multi sampling is enabled
	};


	/**
	 * WindowAttributes
	 *
	 * Set of window attributes used when creating a new opengl window
	 */
	struct WindowSettings
	{
		WindowSettings()  = default;
		~WindowSettings() = default;

		std::string title;						// Name of the window
		int x = SDL_WINDOWPOS_CENTERED;			// Horizontal position of the window
		int y = SDL_WINDOWPOS_CENTERED;			// Vertical position of the window
		int width  = 512;						// Width of the window
		int height = 512;						// Height of the window
		bool borderless = false;				// If the window has no borders
		bool resizable = false;					// If the window is resizable
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
	 * createContext
	 * 
	 * Creates a new opengl context that is associated with the incoming window
	 * Also makes the current context current
	 * @return: the created context, nullptr if not successful
	 */
	SDL_GLContext createContext(SDL_Window& window, bool vSync = true);


	/**
	 * createWindow
	 *
	 * Creates a new opengl window using the parameters specified
	 * @return: the create window, nullptr if not successful
	 */
	SDL_Window* createWindow(const WindowSettings& settings);


	/**
	* swap
	*
	* Swaps buffer for the window with what was drawn in the currently active context
	* Only possible when double buffering is enabled
	* @param window: the window to swap buffers for
	*/
	void swap(SDL_Window& window);


	/**
	 * deleteContext
	 *
	 * Deletes an opengl context, making it unavailable for OpenGl operations
	 */
	void deleteContext(SDL_GLContext context);


	/**
	 * destroyWindow
	 * 
	 * Destroys an OpenGL window
	 */
	void destroyWindow(SDL_Window& window);


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
