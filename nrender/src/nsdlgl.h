#pragma once

// External Includes
#include <string>

namespace opengl
{

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
