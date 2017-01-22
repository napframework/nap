// Local Includes
#include "openglrenderer.h"

// External Includes
#include <nopengl.h>

namespace nap
{
	// GL Render window constructor
	OpenGLRenderWindow::OpenGLRenderWindow(const RenderWindowSettings& settings, opengl::Window* window) : RenderWindow(settings)
	{
		// Set window
		mWindow.reset(window);
	}


	// Returns the actual opengl window
	void* OpenGLRenderWindow::getWindow() const
	{
		return mWindow->getWindow();
	}


	// Returns the actual opengl context
	void* OpenGLRenderWindow::getContext() const
	{
		return mWindow->getContext();
	}


	// return window
	opengl::Window* OpenGLRenderWindow::getContainer() const
	{
		return mWindow.get();
	}


	// Set window title
	void OpenGLRenderWindow::setTitle(const std::string& title)
	{
		opengl::setWindowTitle(*mWindow, title.c_str());
	}


	// Set opengl window position
	void OpenGLRenderWindow::setPosition(const glm::ivec2& position)
	{
		opengl::setWindowPosition(*mWindow, position.x, position.y);
	}


	// Set opengl window size 
	void OpenGLRenderWindow::setSize(const glm::ivec2& size)
	{
		opengl::setWindowSize(*mWindow, size.x, size.y);
		glViewport(0, 0, size.x, size.y);
	}


	// Set opengl sync
	void OpenGLRenderWindow::setSync(bool value)
	{
		opengl::setVSync(*mWindow, value);
	}


	// Show opengl window
	void OpenGLRenderWindow::showWindow()
	{
		opengl::showWindow(*mWindow);
	}


	// Hide opengl window
	void OpenGLRenderWindow::hideWindow()
	{
		opengl::hideWindow(*mWindow);
	}


	// Swap OpenGL buffers
	void OpenGLRenderWindow::swap()
	{
		opengl::swap(*mWindow);
	}


	// Make this window's context current 
	void OpenGLRenderWindow::makeCurrent()
	{
		opengl::makeCurrent(*mWindow);
	}


	// Initializes the opengl subsystem
	bool OpenGLRenderer::preInit()
	{
		if (!opengl::initVideo())
			return false;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.dubbleBuffer = true;
		attrs.versionMinor = 2;
		attrs.versionMajor = 3;
		opengl::setAttributes(attrs);

		// Success
		return true;
	}


	// Create an opengl window
	RenderWindow* OpenGLRenderer::createRenderWindow(const RenderWindowSettings& settings)
	{
		// Convert settings to gl settings
		opengl::WindowSettings gl_window_settings;
		gl_window_settings.borderless = settings.borderless;
		gl_window_settings.resizable = settings.resizable;
		
		// Check if we need to share context information
		if (settings.sharedWindow != nullptr)
		{
			if (!settings.sharedWindow->getTypeInfo().isKindOf(RTTI_OF(OpenGLRenderWindow)))
			{
				nap::Logger::fatal("trying to share a context with non OpenGL type window");
				return false;
			}
			else
			{
				OpenGLRenderWindow* gl_share_window = static_cast<OpenGLRenderWindow*>(settings.sharedWindow);
				assert(gl_share_window != nullptr);
				gl_window_settings.share = gl_share_window->getContainer();
			}
		}

		// Construct new window using these settings
		opengl::Window* new_window = opengl::createWindow(gl_window_settings);
		if (new_window == nullptr)
		{
			nap::Logger::fatal("unable to create new OpenGL render window");
			return false;
		}

		// Construct and return new window
		return new OpenGLRenderWindow(settings, new_window);
	}


	// Initialize glew subsystem
	bool OpenGLRenderer::postInit()
	{
		if (!opengl::init())
		{
			nap::Logger::fatal("unable to initialize OpenGL glew subsystem");
			return false;
		}
		return true;
	}


	// Closes all opengl systems
	void OpenGLRenderer::shutdown()
	{
		opengl::shutdown();
	}

}

RTTI_DEFINE(nap::OpenGLRenderer)
RTTI_DEFINE(nap::OpenGLRenderWindow)