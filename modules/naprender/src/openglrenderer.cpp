// Local Includes
#include "openglrenderer.h"

// External Includes
#include <nopengl.h>

namespace nap
{
	// GL Render window constructor
	OpenGLRenderWindow::OpenGLRenderWindow(const RenderWindowSettings& settings, std::unique_ptr<opengl::Window> window) : 
		RenderWindow(settings),
		mWindow(std::move(window)),
		mBackbuffer(new opengl::BackbufferRenderTarget())
	{
		setSize(glm::vec2(settings.width, settings.height));
	}


	// Returns the actual opengl window
	void* OpenGLRenderWindow::getNativeWindow() const
	{
		return mWindow->getWindow();
	}


	// Returns the backbuffer
	void* OpenGLRenderWindow::getBackbuffer() const
	{
		return mBackbuffer.get();
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
		// Ensure position is not the same
		int x, y;
		opengl::getWindowPosition(*mWindow, x, y);
		if (x == position.x && y == position.y)
			return;

		// Update position
		opengl::setWindowPosition(*mWindow, position.x, position.y);
	}


	// Set opengl window size 
	void OpenGLRenderWindow::setSize(const glm::ivec2& size)
	{
		mBackbuffer->setSize(size);
		mResizeEvent.trigger(WindowResizeEvent(size));

		// Ensure sizes are not the same
		int width, height;
		opengl::getWindowSize(*mWindow, width, height);
		if (width == size.x && height == size.y)
			return;

		// Otherwise set
		opengl::setWindowSize(*mWindow, size.x, size.y);
	}


	// Get the window size
	const glm::ivec2 OpenGLRenderWindow::getSize() const
	{
		int width, height;
		opengl::getWindowSize(*mWindow, width, height);
		
		return glm::ivec2(width, height);
	}


	// Update render viewport
	void OpenGLRenderWindow::setViewport(const glm::ivec2& viewport)
	{
		opengl::setViewport(viewport.x, viewport.y);
	}


	// Set opengl sync
	void OpenGLRenderWindow::setSync(bool value)
	{
		opengl::setVSync(*mWindow, value);
	}


	// Makes the window go full screen
	void OpenGLRenderWindow::setFullScreen(bool value)
	{
		// Otherwise set
		nap::uint32 full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
		nap::uint32 flag = value ? full_screen_flag : 0;
		SDL_SetWindowFullscreen(mWindow->getWindow(), flag);
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
		opengl::flush();
		opengl::swap(*mWindow);
	}

	// Make this window's context current 
	void OpenGLRenderWindow::makeCurrent()
	{
		opengl::makeCurrent(*mWindow);
	}


	bool OpenGLRenderer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(opengl::initVideo(), "Failed to init SDL"))
			return false;

		// Set GL Attributes
		opengl::Attributes attrs;
		attrs.doubleBuffer = true;
		attrs.versionMinor = 3;
		attrs.versionMajor = 3;
		attrs.enableMultiSampling = true;
		attrs.multiSampleSamples = 8;
#if _DEBUG
		attrs.debug = true;
#endif
		opengl::setAttributes(attrs);

		RenderWindowSettings settings;
		settings.visible = false;
		mPrimaryWindow = createRenderWindow(settings, errorState);

		if (!errorState.check(opengl::init(), "Failed to init OpenGL"))
			return false;

		mPrimaryWindow->makeCurrent();

		return true;
	}
	

	// Create an opengl window
	std::unique_ptr<RenderWindow> OpenGLRenderer::createRenderWindow(const RenderWindowSettings& settings, utility::ErrorState& errorState)
	{
		// Convert settings to gl settings
		opengl::WindowSettings gl_window_settings;
		gl_window_settings.borderless = settings.borderless;
		gl_window_settings.resizable = settings.resizable;
		gl_window_settings.visible = settings.visible;
		gl_window_settings.width = settings.width;
		gl_window_settings.height = settings.height;
		gl_window_settings.title = settings.title;

		if (mPrimaryWindow != nullptr)
			gl_window_settings.share = static_cast<OpenGLRenderWindow*>(mPrimaryWindow.get())->getContainer();

		// Construct new window using these settings
		std::unique_ptr<opengl::Window> new_window = opengl::createWindow(gl_window_settings, errorState);
		if (new_window == nullptr)
			return nullptr;

		// Construct and return new window
		return std::make_unique<OpenGLRenderWindow>(settings, std::move(new_window));
	}


	// Closes all opengl systems
	void OpenGLRenderer::shutdown()
	{
		opengl::shutdown();
	}

}

RTTI_DEFINE(nap::OpenGLRenderer)
RTTI_DEFINE_BASE(nap::OpenGLRenderWindow)