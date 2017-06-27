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
