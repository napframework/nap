// Local includes
#include "glwindow.h"

// External includes
#include <utility/errorstate.h>
#include <nglutils.h>

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
		options = settings.resizable ? options	| SDL_WINDOW_RESIZABLE : options;
		options = settings.borderless ? options | SDL_WINDOW_BORDERLESS : options;
		options = !settings.visible ? options	| SDL_WINDOW_HIDDEN : options;
        options = settings.highdpi ?  options	| SDL_WINDOW_ALLOW_HIGHDPI : options;

		SDL_Window* new_window = SDL_CreateWindow(	settings.title.c_str(),
													settings.x,
													settings.y,
													settings.width,
													settings.height,
													options);

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", opengl::getSDLError().c_str()))
			return nullptr;

		return new_window;
	}


	// GL Render window constructor
	GLWindow::GLWindow() { }


	GLWindow::~GLWindow()
	{
		if (mContext != nullptr)
			opengl::deleteContext(mContext);
			
		if (mWindow != nullptr)
			opengl::deleteWindow(mWindow);
	}


	void GLWindow::applySettings(const RenderWindowSettings& settings)
	{
		opengl::enableVSync(settings.sync);
		setSize(glm::vec2(settings.width, settings.height));
		opengl::setWindowPosition(mWindow, glm::vec2(settings.x, settings.y));
		opengl::setWindowResizable(mWindow, settings.resizable);
		opengl::setWindowBordered(mWindow, !settings.borderless);
		opengl::setWindowTitle(mWindow, settings.title);
		if (settings.visible)
			showWindow();
		else
			hideWindow();
	}


	// Creates a window with an associated OpenGL context
	bool GLWindow::init(const RenderWindowSettings& settings, GLWindow* sharedWindow, nap::utility::ErrorState& errorState)
	{
		// create the window
		mWindow = createSDLWindow(settings, errorState);
		if (mWindow == nullptr)
			return false;

		// check if settings contains a shared context
		if (sharedWindow != nullptr)
		{
			// Activate context if necessary
			if (opengl::getCurrentContext() != sharedWindow->getContext())
			{
				opengl::makeCurrent(sharedWindow->getNativeWindow(), sharedWindow->getContext());
			}

			// Enable sharing
			opengl::setAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		}
		else
		{
			opengl::setAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
		}

		mContext = opengl::createContext(mWindow);
		if (!errorState.check(mContext != nullptr, "Failed to create OpenGL Context: %s", SDL_GetError()))
		{
			opengl::deleteWindow(mWindow);
			mWindow = nullptr;
			return false;
		}

		// Disable / Enable v-sync
		opengl::enableVSync(settings.sync);

		setSize(glm::vec2(settings.width, settings.height));
		return true;
	}


	// Returns the actual opengl window
	SDL_Window* GLWindow::getNativeWindow() const
	{
		return mWindow;
	}


	// Returns the backbuffer
	const opengl::BackbufferRenderTarget& GLWindow::getBackbuffer() const
	{
		return mBackbuffer;
	}


	opengl::BackbufferRenderTarget& GLWindow::getBackbuffer()
	{
		return mBackbuffer;
	}

	// Returns the actual opengl context
	SDL_GLContext GLWindow::getContext() const
	{
		return mContext;
	}


	// Set window title
	void GLWindow::setTitle(const std::string& title)
	{
		opengl::setWindowTitle(mWindow, title);
	}


	// Set opengl window position
	void GLWindow::setPosition(const glm::ivec2& position)
	{
		opengl::setWindowPosition(mWindow, position);
	}


	void GLWindow::setShowMouseCursor(const bool show)
	{
		SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
	}


	// Set opengl window size 
	void GLWindow::setSize(const glm::ivec2& size)
	{
		// Set set of window
		opengl::setWindowSize(mWindow, size);
        
        // Backbuffer can have more pixels than the represented window (OSX / Retina)
        // Get pixel size accordingly
        mBackbuffer.setSize(opengl::getDrawableWindowSize(mWindow));
	}


	// Get the window size
	const glm::ivec2 GLWindow::getSize() const
	{
		return opengl::getWindowSize(mWindow);
	}


	// Makes the window go full screen
	void GLWindow::setFullScreen(bool value)
	{
		opengl::setFullscreen(mWindow, value);
	}


	// Show opengl window
	void GLWindow::showWindow()
	{
		opengl::showWindow(mWindow, true);
		opengl::raiseWindow(mWindow);
	}


	// Hide opengl window
	void GLWindow::hideWindow()
	{
		opengl::showWindow(mWindow, false);
	}


	// Swap OpenGL buffers
	void GLWindow::swap()
	{
		opengl::flush();
		opengl::swap(mWindow);
	}


	// Make this window's context current 
	void GLWindow::makeCurrent()
	{
		// Don't trigger change if context is currently active
		if (opengl::getCurrentContext() == mContext)
			return;

		// Make context current
		opengl::makeCurrent(mWindow, mContext);
	}


	// The window number
	nap::uint32 GLWindow::getNumber() const
	{
		return opengl::getWindowId(getNativeWindow());
	}


	glm::ivec2 GLWindow::getPosition()
	{
		return opengl::getWindowPosition(mWindow);
	}

}

RTTI_DEFINE_BASE(nap::GLWindow)
