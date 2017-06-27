#pragma once

// External Includes
#include <rtti/rtti.h>
#include <string.h>
#include <glm/glm.hpp>
#include "nframebuffer.h"
#include "nwindow.h"

struct SDL_Window;
typedef void *SDL_GLContext;

namespace opengl
{
	class Window;
}

namespace nap
{
	// Forward Declares
	class RenderWindow;

	/**
	* Holds all window launch settings
	* Note that this object is only used when constructing the window
	* Use the size, position and title attributes on the component
	* to position the window
	*/
	struct RenderWindowSettings
	{
	public:
		// Construction / Destruction
		RenderWindowSettings() = default;
		virtual ~RenderWindowSettings() = default;

		std::string		title;						// Name of the window
		int				width			= 512;		// Width of the window
		int				height			= 512;		// Height of the window
		bool			borderless		= false;	// If the window is borderless
		bool			resizable		= true;		// If the window is resizable
		bool			visible			= true;		// If the window is visible or not
	};


	/**
	* Render window base class
	*/
	class RenderWindow
	{
		RTTI_ENABLE()
	public:
		/**
		* 
		*/
		RenderWindow(const RenderWindowSettings& settings, std::unique_ptr<opengl::Window> window);

		/**
		* Default destruction
		*/
		virtual ~RenderWindow() = default;

		/**
		* Only construct using window settings
		*/
		RenderWindow(const RenderWindowSettings& settings) :
			mSettings(settings) {}

		/**
		* Delete copy construction
		*/
		RenderWindow(const RenderWindow& other) = delete;
		RenderWindow& operator=(const RenderWindow& other) = delete;

		/**
		* @return the hardware window handle, nullptr if undefined
		*/
		SDL_Window* getNativeWindow() const;

		opengl::Window* getContainer() const { return mWindow.get(); }

		/**
		* @return the hardware window context, nullptr if undefined
		*/
		SDL_GLContext getContext() const;

		/**
		 *@return the backbuffer
		 */
		opengl::BackbufferRenderTarget* getBackbuffer() const;

		/**
		 * Set the window title
		 * @param title the new window title
		 */
		void setTitle(const std::string& title);

		/**
		 * Set the window position
		 * @param position the window position coordinates in pixels
		 */
		void setPosition(const glm::ivec2& position);

		/**
		 * Set the window size
		 * @param size the new window size in pixels
		 */
		void setSize(const glm::ivec2& size);

		/**
		 * Get the window size
		 */
		const glm::ivec2 getSize() const;

		/* 
		 * Set the window viewport
		 */
		void setViewport(const glm::ivec2& viewport);

		/**
		 * Turns v-sync on / off
		 * @param value if v-sync should be turned on or off
		 */
		void setSync(bool value);

		/**
		 * Makes the window full screen
		 * @param value if the window should be full screen or not
		 */
		void setFullScreen(bool value);

		/**
		 * Show window
		 */
		void showWindow();

		/**
		 * Hide window
		 */
		void hideWindow();

		/**
		 * Swap buffers
		 */
		void swap();

		/**
		 * Make this window active
		 */
		void makeCurrent();

	private:
		RenderWindowSettings mSettings;
		std::unique_ptr<opengl::Window> mWindow = nullptr;
		std::unique_ptr<opengl::BackbufferRenderTarget> mBackbuffer = nullptr;
	};
}
