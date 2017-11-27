#pragma once

// Local Includes
#include "glwindow.h"

// External Includes
#include <rtti/rtti.h>
#include <utility/dllexport.h>

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
		 * @param windowID the ID of the window to create
		 */
		std::shared_ptr<GLWindow> createRenderWindow(const RenderWindowSettings& settings, const std::string& windowID, utility::ErrorState& errorState);

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
		GLWindow& getPrimaryWindow() { return *mPrimaryWindow; }

	private:
		std::shared_ptr<GLWindow>	mPrimaryWindow;			///< Primary Window. This always exists for as long as the Renderer exists.
		std::string					mPrimaryWindowID;		///< When a RenderWindow is bound to the primary window, this contains the ID of the RenderWindow
	};
}
