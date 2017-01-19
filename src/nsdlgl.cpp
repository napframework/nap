// Local Includes
#include "nsdlgl.h"
#include "nglutils.h"

// External Includes
#include <iostream>
#include <assert.h>
#include <GL/glew.h>

namespace opengl
{
	const static int minGLVersionMajor = 3;
	const static int minGLVersionMinor = 2;

	// Sets OpenGL attributes
	void setAttributes(const Attributes& attributes)
	{
		// Set our OpenGL version.
		// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
		// 4.1 is the highest available number on most OSX devices
		int cur_minor = attributes.versionMinor;
		int cur_major = attributes.versionMajor;
		
		// Calculate min required gl version 
		int min_version = minGLVersionMajor * 10 + minGLVersionMinor;
		int cur_version = cur_major * 10 + cur_minor;

		// CLamp based on settings
		if (cur_version < min_version)
		{
			printMessage(MessageType::WARNING, "minimum supported GL version is: %d.%d", minGLVersionMajor, minGLVersionMinor);
			printMessage(MessageType::WARNING, "received: %d.%d", cur_major, cur_minor);
			cur_minor = minGLVersionMinor;
			cur_major = minGLVersionMajor;
		}

		// Set settings
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, cur_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, cur_minor);

		// Set double buffering
		int double_buffer = static_cast<int>(attributes.dubbleBuffer);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, double_buffer);

		// Set multi sample parameters
		if (attributes.enableMultiSampling)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, attributes.multiSampleSamples);
		}
	}


	/**
	* createWindow
	*
	* Creates a new opengl window using the parameters specified
	* @return: the create window, nullptr if not successful
	*/
	static SDL_Window* createSDLWindow(const WindowSettings& settings)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_OPENGL;
		options = settings.resizable  ? options | SDL_WINDOW_RESIZABLE  : options;
		options = settings.borderless ? options | SDL_WINDOW_BORDERLESS : options;

		SDL_Window* new_window = SDL_CreateWindow(settings.title.c_str(),
			settings.x,
			settings.y,
			settings.width,
			settings.height,
			options);

		// Make sure we were able to create oen
		if (new_window == nullptr)
		{	
			printMessage(MessageType::ERROR, "unable to create a new window with name: %s", settings.title.c_str());
			printSDLError();
		}
		return new_window;
	}


	/**
	* createContext
	*
	* Creates a new opengl context that is associated with the incoming window
	* Also makes the current context current
	* @return: the created context, nullptr if not successful
	*/
	static SDL_GLContext createContext(SDL_Window& window, bool vSync)
	{
		SDL_GLContext context = SDL_GL_CreateContext(&window);
		if (context == nullptr)
		{
			printMessage(MessageType::ERROR, "unable to create context for window: %s", SDL_GetWindowTitle(&window));
			printSDLError();
			return nullptr;
		}

		// Enable / Disable v-sync
		// This makes our buffer swap syncronized with the monitor's vertical refresh
		SDL_GL_SetSwapInterval(static_cast<int>(vSync));

		// Print Context Info
		const char* window_title = SDL_GetWindowTitle(&window);
		printMessage(MessageType::INFO, "created context for window: %s", window_title);

		int major_version, minor_version;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version);
		printMessage(MessageType::INFO, "version: %d.%d", major_version, minor_version);

		return context;
	}


	// Creates a window with an associated OpenGL context
	Window* createWindow(const WindowSettings& settings)
	{
		// create the window
		SDL_Window* new_window = createSDLWindow(settings);
		if (new_window == nullptr)
			return nullptr;

		// check if settings contains a shared context
		if (settings.share != nullptr)
		{
			assert(settings.share->getWindow()  != nullptr);
			assert(settings.share->getContext() != nullptr);
			
			// Activate context if necessary
			if (SDL_GL_GetCurrentContext() != settings.share->getContext())
			{
				SDL_GL_MakeCurrent(settings.share->getWindow(), settings.share->getContext());
			}

			// Enable sharing
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		}
		else
		{
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
		}

		// Create context
		SDL_GLContext context = createContext(*new_window, true);
		if (context == nullptr)
			return nullptr;

		// Create window container
		return new Window(settings, new_window, context);
	}


	// Hide selected window
	void hideWindow(Window& window)
	{
		assert(window.getWindow() != nullptr);
		SDL_HideWindow(window.getWindow());
	}


	// Show selected window
	void showWindow(Window& window)
	{
		assert(window.getWindow() != nullptr);
		SDL_ShowWindow(window.getWindow());
	}


	// Updates window position
	void setWindowPosition(Window& window, int x, int y)
	{
		assert(window.getWindow() != nullptr);
		SDL_SetWindowPosition(window.getWindow(), x, y);
	}


	// Updates the window title
	void setWindowTitle(Window& window, const std::string& title)
	{
		assert(window.getWindow() != nullptr);
		SDL_SetWindowTitle(window.getWindow(), title.c_str());
	}


	// Sets the window size
	void setWindowSize(Window& window, int width, int height)
	{
		assert(window.getWindow() != nullptr);
		SDL_SetWindowSize(window.getWindow(), width, height);
	}

	// Swaps buffer for the window
	// This action is relative to currently active drawing context
	void swap(Window& window)
	{
		SDL_GL_SwapWindow(window.getWindow());
	}


	// Initializes SDL's video subsystem
	bool initVideo()
	{
		// Initialize SDL's Video subsystem
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printMessage(MessageType::ERROR, "failed to init SDL video subsystem");
			printSDLError();
			return false;
		}
		return true;
	}


	// Initializes the OpenGL Glew wrapper
	// This call will only succeed when OpenGL has a valid current context
	bool init()
	{
#ifndef __APPLE__
		glewExperimental = GL_TRUE;
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			printMessage(MessageType::ERROR, "glew init failed with error code %d:", error);
			return false;
		}
#endif // !__APPLE__

		printMessage(MessageType::INFO, "initialized glew successfully");
		printMessage(MessageType::INFO, "vendor: %s", glGetString(GL_VENDOR));
		printMessage(MessageType::INFO, "shading language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		return true;
	}


	// Shuts down SDL, make sure to 
	void shutdown()
	{
		SDL_Quit();
	}


	// Returns the last SDL error
	std::string getSDLError()
	{
		return std::string(SDL_GetError());
	}


	// Prints the last SDL error
	void printSDLError()
	{
		printMessage(MessageType::ERROR, getSDLError().c_str());
	}

}	// opengl