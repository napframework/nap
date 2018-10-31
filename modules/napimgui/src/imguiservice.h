#pragma once

// External includes
#include <nap/service.h>
#include <utility/dllexport.h>
#include <renderwindow.h>

namespace nap
{
	// Forward Declares
	class RenderService;
	class GuiWindow;

	/**
	 * This service manages the global ImGui state.
	 * Use setWindow() to select the window to draw the GUI on to.
	 * By default the GUI is drawn to the primary window, as defined by the renderer.
	 * Make sure to call draw() inside your application to render the gui to the window. 
	 * When doing so make sure the window that you selected is active, otherwise the GUI will not appear.
	 * When there is no actively selected window call draw() after making the primary window active.
	 * The service automatically creates a new GUI frame before calling update.
	 */
	class NAPAPI IMGuiService : public Service
	{
		RTTI_ENABLE(Service)
		friend class GuiWindow;
	public:
		/**
		 *	Default constructor
		 */
		IMGuiService(ServiceConfiguration* configuration);

		/**
		 * Draws the all the GUI elements to screen
		 * You need to call this just before swapping buffers for the primary window
		 */
		void draw();

		/**
		 * Explicitly set the window that is used for drawing the GUI elements
		 * When no window is specified the system uses the primary window to draw gui elements
		 * Preferably call this on init() of the application.
		 * @param window the window to use for drawing the GUI elements
		 */
		void setWindow(nap::ResourcePtr<RenderWindow> window);

	protected:
		/**
		 * Initializes the IMGui library. This will also create all associated gui devices objects
		 * Note that a GUI can only be associated with the primary window (for now).
		 * @param error contains the error message if the lib could not be initialized correctly
		 * @return if the lib was initialized successfully
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * ImGui depends on the renderer
		 * @param dependencies the type of services this service depends on
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Allows IMGUI to draw a new frame. This is called automatically by core in the app loop
		 * @param deltaTime the time in seconds between ticks
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Disables the imgui library
		 */
		virtual void shutdown() override;

	private:
		RenderService*				mRenderer = nullptr;			///< The rendered used by IMGUI
		ResourcePtr<RenderWindow>	mUserWindow = nullptr;			///< User selected GUI window, defaults to primary window
	};
}