#pragma once

// External includes
#include <nap/service.h>
#include <utility/dllexport.h>

namespace nap
{
	// Forward Declares
	class RenderService;
	class GuiWindow;

	/**
	 * This service manages the global ImGui state
	 * Note that (for now) GUIS are only available for the primary window
	 * Make sure to call render() inside the render callback inside your application
	 * to render the updated GUI to your primary window.
	 * The service automatically creates a new GUI frame before calling update
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
		RenderService*				mRenderer = nullptr;	///< The rendered used by IMGUI
	};
}