#pragma once

#include "nbackbufferrendertarget.h"
#include <rtti/rtti.h>

// External Includes
#include <string.h>
#include <glm/glm.hpp>
#include <SDL_video.h>
#include <nap/configure.h>
#include <utility/dllexport.h>

struct SDL_Window;
typedef void *SDL_GLContext;

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}

	// Forward Declares
	class GLWindow;

	/**
	* Holds all window launch settings
	* Note that this object is only used when constructing the window
	* Use the size, position and title attributes on the component
	* to position the window
	*/
	struct NAPAPI RenderWindowSettings
	{
	public:
		// Construction / Destruction
		RenderWindowSettings() = default;
		virtual ~RenderWindowSettings() = default;

		std::string		title;										// Name of the window
		int				x				= SDL_WINDOWPOS_CENTERED;	// Position
		int				y				= SDL_WINDOWPOS_CENTERED;	// Position
		int				width			= 512;						// Width of the window
		int				height			= 512;						// Height of the window
		bool			borderless		= false;					// If the window is borderless
		bool			resizable		= true;						// If the window is resizable
		bool			visible			= true;						// If the window is visible or not
		bool			sync			= true;						// If v-sync is turned on for the window
	};


	/**
	* Render window base class
	*/
	class NAPAPI GLWindow final
	{
		RTTI_ENABLE()
	public:

		GLWindow();
		~GLWindow();

		/**
		* Delete copy construction
		*/
		GLWindow(const GLWindow& other) = delete;
		GLWindow& operator=(const GLWindow& other) = delete;

		bool init(const RenderWindowSettings& settings, GLWindow* sharedWindow, utility::ErrorState& errorState);

		/**
		* @return the hardware window handle, nullptr if undefined
		*/
		SDL_Window* getNativeWindow() const;

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

		/**
		 *	Returns the window number
		 */
		uint32 getNumber() const;

	private:
		std::unique_ptr<opengl::BackbufferRenderTarget> mBackbuffer = nullptr;
		SDL_Window*										mWindow = nullptr;		// Actual GL window
		SDL_GLContext									mContext = nullptr;		// GL Context
	};
}
