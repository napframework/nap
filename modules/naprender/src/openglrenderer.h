#pragma once

// Local Includes
#include "renderer.h"

// External Includes
#include <nsdlgl.h>

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
		RTTI_ENABLE_DERIVED_FROM(RenderWindow)
	public:
		// Constructor
		OpenGLRenderWindow(const RenderWindowSettings& settings, opengl::Window* window);

		/**
		 * @return SDL_Window*
		 */
		virtual void* getWindow() const override;

		/**
		 * @return SDL_GLContext (=void*)
		 */
		virtual void* getContext() const override;

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
		* Turns v-sync on / off
		* @param value if v-sync should be turned on or off
		*/
		virtual void setSync(bool value) override;

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

	private:
		std::unique_ptr<opengl::Window> mWindow = nullptr;
	};


	/**
	 * OpenGL renderer
	 * Handles initialization of OpenGL, the video system and creation of windows
	 */
	class OpenGLRenderer : public Renderer
	{
		RTTI_ENABLE_DERIVED_FROM(Renderer)
	public:
		/**
		 * Initializes the video subsystem
		 * @return if initialization of subsystem was successful
		 */
		virtual bool preInit() override;

		/**
		 * Creates an opengl render window using the settings provided
		 * @return the opengl render window, nullptr if unsuccessful
		 */
		virtual RenderWindow* createRenderWindow(const RenderWindowSettings& settings) override;

		/**
		 * Initializes GLEW
		 * @return if glew has been initialized successfully
		 */
		virtual bool postInit() override;

		/**
		 * Closes all active opengl systems
		 */
		virtual void shutdown() override;

	};
}

RTTI_DECLARE(nap::OpenGLRenderer)
RTTI_DECLARE_BASE(nap::OpenGLRenderWindow)