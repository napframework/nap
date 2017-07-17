#pragma once

// Local Includes
#include <renderwindow.h>

// External Includes
#include <rtti/rtti.h>
#include <renderattributes.h>
#include <nap/dllexport.h>

namespace nap
{
	/**
	 * Renderer Base Class
	 * Derived classed implement specific hardware rendering
	 * options. Every renderer renders a set of objects to 
	 * a specific render target. The render target initialization
	 * and binding is handled by that target
	 */
	class NAPAPI Renderer final
	{
		RTTI_ENABLE()
	public:
		/**
		 * Delete copy constructor
		 */
		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

		/**
		 * Default constructor and destructor
		 */
		~Renderer() = default;
		Renderer() = default;

		/**
		 * Called to create a window
		 * Create a window with associated render context
		 * @return the new render window or nullptr if unsuccessful
		 * @param settings the window settings used to create the window
		 */
		std::unique_ptr<RenderWindow> createRenderWindow(const RenderWindowSettings& settings, utility::ErrorState& errorState);

		/**
		 * Initialize the renderer
		 */
		bool init(utility::ErrorState& errorState);

		/**
		 * Called when the renderer needs to shut down
		 */
		void shutdown();

		/**
		 * Get the primary window (i.e. the window that was used to init OpenGL against)
		 */
		RenderWindow& getPrimaryWindow() { return *mPrimaryWindow; }

	private:
		std::unique_ptr<RenderWindow> mPrimaryWindow;
	};
}
