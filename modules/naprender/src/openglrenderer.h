#pragma once

// Local Includes
#include "renderer.h"

// External Includes
#include <nsdlgl.h>
#include "nframebuffer.h"

namespace nap
{
	// Forward Declares
	class OpenGLRenderer;

	/**
	 * Defines an OpenGL render window
	 * This window owns the window passed in
	 * The window is destructed on deletion
	 */
	class OpenGLRenderWindow : public RenderWindow
	{
		RTTI_ENABLE(RenderWindow)
	public:
		// Constructor
		OpenGLRenderWindow(const RenderWindowSettings& settings, std::unique_ptr<opengl::Window> window);

		/**
		 * @return SDL_Window*
		 */
		virtual void* getNativeWindow() const override;

		/**
		 * @return SDL_GLContext (=void*)
		 */
		virtual void* getContext() const override;

		/**
		*@return the backbuffer
		*/
		virtual void* getBackbuffer() const override;

		/**
		 * @return the OpenGL window container
		 * Holds both the window and context
		 */
		opengl::Window* getContainer() const;

		/**
		* Set the window title
		* @param title the new window title
		*/
		virtual void setTitle(const std::string& title) override;

		/**
		* Set the window position
		* @param position the window position coordinates in pixels
		*/
		virtual void setPosition(const glm::ivec2& position) override;

		/**
		* Set the window size
		* @param size the new window size in pixels
		*/
		virtual void setSize(const glm::ivec2& size) override;

		/**
		 * Get the window size
		 */
		virtual const glm::ivec2 getSize() const override;
		
		/**
		 * Set the render viewport
		 */
		virtual void setViewport(const glm::ivec2& viewport) override;

		/**
		* Turns v-sync on / off
		* @param value if v-sync should be turned on or off
		*/
		virtual void setSync(bool value) override;

		/**
		 * Makes the window full screen
		 */
		virtual void setFullScreen(bool value) override;

		/**
		* Show window
		*/
		virtual void showWindow() override;

		/**
		* Hide window
		*/
		virtual void hideWindow() override;

		/**
		 * Swap buffers
		 */
		virtual void swap() override;

		/**
		 * Make this window's context active
		 */
		virtual void makeCurrent() override;

		bool handleEvent(const SDL_Event& event);

	private:
		std::unique_ptr<opengl::Window> mWindow = nullptr;
		std::unique_ptr<opengl::BackbufferRenderTarget> mBackbuffer = nullptr;
	};


	/**
	 * OpenGL renderer
	 * Handles initialization of OpenGL, the video system and creation of windows
	 */
	class OpenGLRenderer : public Renderer
	{
		RTTI_ENABLE(Renderer)
	public:
		/**
		 * Initialize the renderer
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Creates an opengl render window using the settings provided
		 * @return the opengl render window, nullptr if unsuccessful
		 */
		virtual std::unique_ptr<RenderWindow> createRenderWindow(const RenderWindowSettings& settings, utility::ErrorState& errorState) override;

		/**
		 * Closes all active opengl systems
		 */
		virtual void shutdown() override;

		/**
		 * Get the primary window (i.e. the window that was used to init OpenGL against)
		 */
		virtual RenderWindow& getPrimaryWindow() override { return *mPrimaryWindow; }

	private:
		std::unique_ptr<RenderWindow> mPrimaryWindow;
	};
}
