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
	void setVSync(bool value)
	{
		SDL_GL_SetSwapInterval(static_cast<int>(value));
	}

	void openGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
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

}	// opengl
