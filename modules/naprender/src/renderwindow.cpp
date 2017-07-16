// Local includes
#include "renderwindow.h"
#include "nsdlgl.h"
#include "nglutils.h"
#include "nap\configure.h"
#include "utility\errorstate.h"

namespace nap
{
	/**
	* createWindow
	*
	* Creates a new opengl window using the parameters specified
	* @return: the create window, nullptr if not successful
	*/
	static SDL_Window* createSDLWindow(const RenderWindowSettings& settings, nap::utility::ErrorState& errorState)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_OPENGL;
		options = settings.resizable ? options | SDL_WINDOW_RESIZABLE : options;
		options = settings.borderless ? options | SDL_WINDOW_BORDERLESS : options;
		options = !settings.visible ? options | SDL_WINDOW_HIDDEN : options;

		SDL_Window* new_window = SDL_CreateWindow(	settings.title.c_str(),
													settings.x,
													settings.y,
													settings.width,
													settings.height,
													options);

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", SDL_GetError()))
			return nullptr;

		return new_window;
	}


	// GL Render window constructor
	RenderWindow::RenderWindow() :
		mBackbuffer(new opengl::BackbufferRenderTarget())
	{
	}


	RenderWindow::~RenderWindow()
	{
		if (mContext != nullptr)
			SDL_GL_DeleteContext(mContext);

		if (mWindow != nullptr)
			SDL_DestroyWindow(mWindow);
	}


	// Creates a window with an associated OpenGL context
	bool RenderWindow::init(const RenderWindowSettings& settings, RenderWindow* sharedWindow, nap::utility::ErrorState& errorState)
	{
		// create the window
		mWindow = createSDLWindow(settings, errorState);
		if (mWindow == nullptr)
			return false;

		// check if settings contains a shared context
		if (sharedWindow != nullptr)
		{
			// Activate context if necessary
			if (SDL_GL_GetCurrentContext() != sharedWindow->getContext())
			{
				SDL_GL_MakeCurrent(sharedWindow->getNativeWindow(), sharedWindow->getContext());
			}

			// Enable sharing
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		}
		else
		{
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
		}

		mContext = SDL_GL_CreateContext(mWindow);
		if (!errorState.check(mContext != nullptr, "Failed to create OpenGL Context: %s", SDL_GetError()))
		{
			SDL_DestroyWindow(mWindow);
			mWindow = nullptr;
			return false;
		}

		setSize(glm::vec2(settings.width, settings.height));

		return true;
	}


	// Returns the actual opengl window
	SDL_Window* RenderWindow::getNativeWindow() const
	{
		return mWindow;
	}


	// Returns the backbuffer
	opengl::BackbufferRenderTarget* RenderWindow::getBackbuffer() const
	{
		return mBackbuffer.get();
	}


	// Returns the actual opengl context
	SDL_GLContext RenderWindow::getContext() const
	{
		return mContext;
	}


	// Set window title
	void RenderWindow::setTitle(const std::string& title)
	{
		SDL_SetWindowTitle(mWindow, title.c_str());
	}


	// Set opengl window position
	void RenderWindow::setPosition(const glm::ivec2& position)
	{
		// Ensure position is not the same
		int x, y;
		SDL_GetWindowPosition(mWindow, &x, &y);
		if (x == position.x && y == position.y)
			return;

		// Update position
		SDL_SetWindowPosition(mWindow, position.x, position.y);
	}


	// Set opengl window size 
	void RenderWindow::setSize(const glm::ivec2& size)
	{
		mBackbuffer->setSize(size);

		// Ensure sizes are not the same
		int width, height;
		SDL_GetWindowSize(mWindow, &width, &height);
		if (width == size.x && height == size.y)
			return;

		// Otherwise set
		SDL_SetWindowSize(mWindow, width, height);
	}


	// Get the window size
	const glm::ivec2 RenderWindow::getSize() const
	{
		int width, height;
		SDL_GetWindowSize(mWindow, &width, &height);

		return glm::ivec2(width, height);
	}


	// Makes the window go full screen
	void RenderWindow::setFullScreen(bool value)
	{
		// Otherwise set
		nap::uint32 full_screen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
		nap::uint32 flag = value ? full_screen_flag : 0;
		SDL_SetWindowFullscreen(mWindow, flag);
	}


	// Show opengl window
	void RenderWindow::showWindow()
	{
		SDL_ShowWindow(mWindow);
	}


	// Hide opengl window
	void RenderWindow::hideWindow()
	{
		SDL_HideWindow(mWindow);
	}


	// Swap OpenGL buffers
	void RenderWindow::swap()
	{
		glFlush();
		SDL_GL_SwapWindow(mWindow);
	}

	// Make this window's context current 
	void RenderWindow::makeCurrent()
	{
		// Don't trigger change if context is currently active
		if (SDL_GL_GetCurrentContext() == mContext)
			return;

		// Make context current
		SDL_GL_MakeCurrent(mWindow, mContext);
	}

	// The window number
	nap::uint32 RenderWindow::getNumber() const
	{
		return opengl::getWindowId(getNativeWindow());
	}

}

RTTI_DEFINE_BASE(nap::RenderWindow)