// Local includes
#include "renderwindow.h"
#include "nsdlgl.h"
#include "nglutils.h"
#include "nap\configure.h"

namespace nap
{
	// GL Render window constructor
	RenderWindow::RenderWindow(const RenderWindowSettings& settings, std::unique_ptr<opengl::Window> window) :
		mSettings(settings),
		mWindow(std::move(window)),
		mBackbuffer(new opengl::BackbufferRenderTarget())
	{
		setSize(glm::vec2(settings.width, settings.height));
	}


	// Returns the actual opengl window
	SDL_Window* RenderWindow::getNativeWindow() const
	{
		return mWindow->getWindow();
	}


	// Returns the backbuffer
	opengl::BackbufferRenderTarget* RenderWindow::getBackbuffer() const
	{
		return mBackbuffer.get();
	}


	// Returns the actual opengl context
	SDL_GLContext RenderWindow::getContext() const
	{
		return mWindow->getContext();
	}


	// Set window title
	void RenderWindow::setTitle(const std::string& title)
	{
		opengl::setWindowTitle(*mWindow, title.c_str());
	}


	// Set opengl window position
	void RenderWindow::setPosition(const glm::ivec2& position)
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
	void RenderWindow::setSize(const glm::ivec2& size)
	{
		mBackbuffer->setSize(size);

		// Ensure sizes are not the same
		int width, height;
		opengl::getWindowSize(*mWindow, width, height);
		if (width == size.x && height == size.y)
			return;

		// Otherwise set
		opengl::setWindowSize(*mWindow, size.x, size.y);
	}


	// Get the window size
	const glm::ivec2 RenderWindow::getSize() const
	{
		int width, height;
		opengl::getWindowSize(*mWindow, width, height);

		return glm::ivec2(width, height);
	}


	// Update render viewport
	void RenderWindow::setViewport(const glm::ivec2& viewport)
	{
		opengl::setViewport(viewport.x, viewport.y);
	}


	// Set opengl sync
	void RenderWindow::setSync(bool value)
	{
		opengl::setVSync(*mWindow, value);
	}


	// Makes the window go full screen
	void RenderWindow::setFullScreen(bool value)
	{
		// Otherwise set
		nap::uint32 full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
		nap::uint32 flag = value ? full_screen_flag : 0;
		SDL_SetWindowFullscreen(mWindow->getWindow(), flag);
	}


	// Show opengl window
	void RenderWindow::showWindow()
	{
		opengl::showWindow(*mWindow);
	}


	// Hide opengl window
	void RenderWindow::hideWindow()
	{
		opengl::hideWindow(*mWindow);
	}


	// Swap OpenGL buffers
	void RenderWindow::swap()
	{
		opengl::flush();
		opengl::swap(*mWindow);
	}

	// Make this window's context current 
	void RenderWindow::makeCurrent()
	{
		opengl::makeCurrent(*mWindow);
	}

}

RTTI_DEFINE_BASE(nap::RenderWindow)