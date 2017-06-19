// Local Includes
#include "nsdlgl.h"
#include "nglutils.h"
#include "utility/errorstate.h"

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
		int double_buffer = static_cast<int>(attributes.doubleBuffer);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, double_buffer);

		// Set multi sample parameters
		if (attributes.enableMultiSampling)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, attributes.multiSampleSamples);
		}

		// Enable debug
		if (attributes.debug)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}		
	}


	/**
	* createWindow
	*
	* Creates a new opengl window using the parameters specified
	* @return: the create window, nullptr if not successful
	*/
	static SDL_Window* createSDLWindow(const WindowSettings& settings, nap::utility::ErrorState& errorState)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_OPENGL;
		options = settings.resizable  ? options | SDL_WINDOW_RESIZABLE  : options;
		options = settings.borderless ? options | SDL_WINDOW_BORDERLESS : options;
		options = !settings.visible ? options | SDL_WINDOW_HIDDEN : options;

		SDL_Window* new_window = SDL_CreateWindow(settings.title.c_str(),
			settings.x,
			settings.y,
			settings.width,
			settings.height,
			options);

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", getSDLError().c_str()))
			return nullptr;

		return new_window;
	}


	/**
	* createContext
	*
	* Creates a new opengl context that is associated with the incoming window
	* Also makes the current context current
	* @return: the created context, nullptr if not successful
	*/
	static SDL_GLContext createContext(SDL_Window& window, bool vSync, nap::utility::ErrorState& errorState)
	{
		SDL_GLContext context = SDL_GL_CreateContext(&window);
		if (!errorState.check(context != nullptr, "Failed to create OpenGL Context: %s", getSDLError().c_str()))
			return false;

		// Print Context Info
		const char* window_title = SDL_GetWindowTitle(&window);
		printMessage(MessageType::INFO, "created context for window: %s", window_title);

		int major_version, minor_version;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version);
		printMessage(MessageType::INFO, "version: %d.%d", major_version, minor_version);

		int rs, gs, bs, ds, as, mb, ms, ss;
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &rs);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &gs);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &bs);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &ds);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &as);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &mb);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &ms);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &ss);

		return context;
	}


	// Creates a window with an associated OpenGL context
	std::unique_ptr<Window> createWindow(const WindowSettings& settings, nap::utility::ErrorState& errorState)
	{
		// create the window
		SDL_Window* new_window = createSDLWindow(settings, errorState);
		if (new_window == nullptr)
			return nullptr;

		// check if settings contains a shared context
		if (settings.share != nullptr)
		{
			assert(settings.share->getWindow() != nullptr);
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
		SDL_GLContext context = createContext(*new_window, true, errorState);
		if (context == nullptr)
			return nullptr;

		// Create window container
		return std::make_unique<Window>(settings, new_window, context);
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


	// Returns the window position
	void getWindowPosition(Window& window, int &x, int &y)
	{
		assert(window.getWindow() != nullptr);
		SDL_GetWindowPosition(window.getWindow(), &x, &y);
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


	void getWindowSize(Window& window, int& width, int& height)
	{
		assert(window.getWindow() != nullptr);
		SDL_GetWindowSize(window.getWindow(), &width, &height);
	}

	// Turn v-sync on / off
	void setVSync(Window& window, bool value)
	{
		SDL_GL_SetSwapInterval(static_cast<int>(value));
	}


	// Swaps buffer for the window
	// This action is relative to currently active drawing context
	void swap(Window& window)
	{
		SDL_GL_SwapWindow(window.getWindow());
	}


	// Make this window's context current
	void makeCurrent(Window& window)
	{
		// Don't trigger change if context is currently active
		if (SDL_GL_GetCurrentContext() == window.getContext())
			return;

		// Make context current
		SDL_GL_MakeCurrent(window.getWindow(), window.getContext());
	}

    // TODO: Not sure why __stdcall is used, making this work on 'the other' compilers
    #ifdef _MSC_VER
        #define __STDCALL __stdcall
    #else
        #define __STDCALL
    #endif

	// OpenGL message callback; invoked when debug is enabled on the OpenGL context
	void __STDCALL openGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		std::string readable_type;
		MessageType message_type = MessageType::INFO;

		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			readable_type = "Error";
			message_type = MessageType::ERROR;
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			readable_type = "Deprecated Behaviour";
			message_type = MessageType::WARNING;
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			readable_type = "Undefined Behaviour";
			message_type = MessageType::WARNING;
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			readable_type = "Portability";
			message_type = MessageType::WARNING;
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			readable_type ="Performance";
			message_type = MessageType::WARNING;
			break;
		case GL_DEBUG_TYPE_OTHER:
			readable_type = "Other";
			message_type = MessageType::INFO;
			break;
		default:
			readable_type = "Unknown";
			message_type = MessageType::INFO;
			break;
		}

		std::string readable_severity;
		switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:
			readable_severity = "Low";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			readable_severity = "Medium";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			readable_severity = "High";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			readable_severity = "Notification";
			break;
		default:
			readable_severity = "Unknown";
			break;
		}

		printMessage(message_type, "[OpenGL Debug][%s][%s] %s", readable_severity.c_str(), readable_type.c_str(), message);
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
//#ifndef __APPLE__
		glewExperimental = GL_TRUE;
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			printMessage(MessageType::ERROR, "glew init failed with error code %d:", error);
			return false;
		}
//#endif // !__APPLE__

		printMessage(MessageType::INFO, "initialized glew successfully");
		printMessage(MessageType::INFO, "vendor: %s", glGetString(GL_VENDOR));
		printMessage(MessageType::INFO, "shading language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		// Check whether debug flag is set
		int context_flags;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &context_flags);
		bool debug = (context_flags & SDL_GL_CONTEXT_DEBUG_FLAG) != 0;

		// If we're using a debug context, install the message callback
		if (debug)
		{
			// Set output to synchronous so that setting a breakpoint in the message callback will show the potentially invalid call on the callstack
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

			// Set callback
			glDebugMessageCallback(openGLMessageCallback, nullptr);

			// First enable all messages
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, true);

			// Then disable low severity messages to prevent useless spam of 'informational' messages
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, 0, false);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, false);
		}
			
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
