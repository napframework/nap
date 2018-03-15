// Local Includes
#include "nsdlgl.h"
#include "nglutils.h"

// External Includes
#include <iostream>
#include <assert.h>
#include <GL/glew.h>

namespace opengl
{
	// Turn v-sync on / off
	void enableVSync(bool value)
	{
		SDL_GL_SetSwapInterval(static_cast<int>(value));
	}

	void openGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		std::string readable_type;
		EGLSLMessageType message_type = EGLSLMessageType::Info;

		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			readable_type = "Error";
			message_type = EGLSLMessageType::Error;
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			readable_type = "Deprecated Behaviour";
			message_type = EGLSLMessageType::Warning;
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			readable_type = "Undefined Behaviour";
			message_type = EGLSLMessageType::Warning;
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			readable_type = "Portability";
			message_type = EGLSLMessageType::Warning;
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			readable_type ="Performance";
			message_type = EGLSLMessageType::Warning;
			break;
		case GL_DEBUG_TYPE_OTHER:
			readable_type = "Other";
			message_type = EGLSLMessageType::Info;
			break;
		default:
			readable_type = "Unknown";
			message_type = EGLSLMessageType::Info;
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
			printMessage(EGLSLMessageType::Error, "failed to init SDL video subsystem");
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
			printMessage(EGLSLMessageType::Error, "glew init failed with error code %d:", error);
			return false;
		}
//#endif // !__APPLE__

		printMessage(EGLSLMessageType::Info, "initialized glew successfully");
		printMessage(EGLSLMessageType::Info, "vendor: %s", glGetString(GL_VENDOR));
		printMessage(EGLSLMessageType::Info, "shading language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

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


	void deleteContext(SDL_GLContext context)
	{
		SDL_GL_DeleteContext(context);
	}


	void deleteWindow(SDL_Window* window)
	{
		SDL_DestroyWindow(window);
	}


	void setWindowResizable(SDL_Window* window, bool resizable)
	{
		// TODO: This isn't supported on < SDL 2.05 versions
		// Either we decide to only support those versions of SDL
		// But that will exclude the current Ubuntu 16.04 version
		// SDL_SetWindowResizable(window, (SDL_bool)resizable);
	}


	void setWindowBordered(SDL_Window* window, bool hasBorders)
	{
		SDL_SetWindowBordered(window, (SDL_bool)hasBorders);
	}


	void setWindowTitle(SDL_Window* window, const std::string& name)
	{
		SDL_SetWindowTitle(window, name.c_str());
	}


	SDL_GLContext getCurrentContext()
	{
		return SDL_GL_GetCurrentContext();
	}


	void swap(SDL_Window* window)
	{
		SDL_GL_SwapWindow(window);
	}


	void showWindow(SDL_Window* window, bool show)
	{
		if (show)
		{
			SDL_ShowWindow(window);
			return;
		}
		SDL_HideWindow(window);
	}


	void raiseWindow(SDL_Window* window)
	{
		SDL_RaiseWindow(window);
	}


	void setFullscreen(SDL_Window* window, bool value)
	{
		// Otherwise set
		std::uint32_t full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
		std::uint32_t flag = value ? full_screen_flag : 0;
		SDL_SetWindowFullscreen(window, flag);
	}


	void makeCurrent(SDL_Window* window, SDL_GLContext context)
	{
		SDL_GL_MakeCurrent(window, context);
	}


	void setAttribute(SDL_GLattr attribute, int value)
	{
		SDL_GL_SetAttribute(attribute, value);
	}


	int getAttribute(SDL_GLattr attribute)
	{
		int v(0);
		SDL_GL_GetAttribute(attribute, &v);
		return v;
	}


	SDL_GLContext createContext(SDL_Window* window)
	{
		return SDL_GL_CreateContext(window);
	}


	glm::ivec2 getWindowSize(SDL_Window* window)
	{
		int x, y;
		SDL_GetWindowSize(window, &x, &y);
		return glm::ivec2(x, y);
	}


	glm::ivec2 getScreenSize(int screenIndex)
	{
		SDL_DisplayMode mode;
		int v = SDL_GetDesktopDisplayMode(screenIndex, &mode);
		return v == 0 ? glm::ivec2(mode.w, mode.h) : glm::ivec2(-1, -1);
	}


	void setWindowSize(SDL_Window* window, const glm::ivec2& size)
	{
		// Ensure sizes are not the same
		glm::ivec2 wsize = opengl::getWindowSize(window);
		if (wsize == size)
			return;
		SDL_SetWindowSize(window, size.x, size.y);
	}


	glm::ivec2 getWindowPosition(SDL_Window* window)
	{
		int x, y;
		SDL_GetWindowPosition(window, &x, &y);
		return glm::ivec2(x, y);
	}


	void setWindowPosition(SDL_Window* window, const glm::ivec2& position)
	{
		// Ensure position is not the same
		glm::ivec2 wpos = opengl::getWindowPosition(window);
		if (position == wpos)
			return;
		// Update position
		SDL_SetWindowPosition(window, position.x, position.y);
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
		printMessage(EGLSLMessageType::Error, getSDLError().c_str());
	}


	// Checks is sdl received an event
	bool pollEvent(opengl::Event& inputEvent)
	{
		return SDL_PollEvent(&inputEvent) > 0;
	}


	// Returns an SDL window based on the given id
	SDL_Window* getWindow(uint32_t  id)
	{
		return SDL_GetWindowFromID((Uint32)(id));
	}


	uint32_t getWindowId(SDL_Window* window)
	{
		return SDL_GetWindowID(window);
	}

}	// opengl
