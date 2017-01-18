#pragma once

// External Includes
#include <SDL.h>
#include <SDL_video.h>
#include <string>
#include <assert.h>

namespace opengl
{
	// Forward Declares
	class Window;

	/**
	* WindowAttributes
	*
	* Set of window attributes used when creating a new opengl window and context
	*/
	struct WindowSettings
	{
		WindowSettings() = default;
		~WindowSettings() = default;

		std::string title;						// Name of the window
		int x = SDL_WINDOWPOS_CENTERED;			// Horizontal position of the window
		int y = SDL_WINDOWPOS_CENTERED;			// Vertical position of the window
		int width = 512;						// Width of the window
		int height = 512;						// Height of the window
		bool borderless = false;				// If the window has no borders
		bool resizable = false;					// If the window is resizable
		Window* share = nullptr;				// The window whose context to share resources with
	};


	/**
	 * An OpenGL window
	 * This object acts as a container for both the window and context
	 * On passing 
	 */
	class Window
	{
	public:
		/**
		 * Constructor for window
		 * @param settings: the window construction settings
		 * @param window: the GL window handle
		 * @param context: the opengl context associated with this window
		 */
		Window(const WindowSettings& settings, SDL_Window* window, SDL_GLContext context) : 
			mSettings(settings),
			mWindow(window),
			mContext(context)							{ }

		/**
		 * Destructor
		 */
		virtual ~Window();

		/**
		 * @return the settings associated with this window on construction
		 */
		const WindowSettings&	getSettings() const		{ return mSettings; }

		/**
		 * @return the OpenGL window
		 */
		SDL_Window*				getWindow() const		{ return mWindow; }

		/**
		 * @return the context associated with this OpenGL window
		 */
		SDL_GLContext			getContext() const		{ return mContext; }

	private:
		WindowSettings			mSettings;				// Window settings
		SDL_Window*				mWindow = nullptr;		// Actual GL window
		SDL_GLContext			mContext = nullptr;		// GL Context
	};
}