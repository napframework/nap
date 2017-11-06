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
	 * Make sure to call 
	 */
	class NAPAPI IMGuiService : public Service
	{
		RTTI_ENABLE(Service)
		friend class GuiWindow;
	public:
		// Default constructor
		IMGuiService() = default;

		/**
		* renders the GUI frame to screen
		* Note that you need to call this just before swapping buffers for the primary window
		*/
		void render();

	protected:
		/**
		* Initializes the IMGui library. This will also create all associated gui devices objects
		* Note that a gui can only be associated with the primary window (for now).
		* @param error contains the error message if the lib could not be initialized correctly
		* @return if the lib was initialized successfully
		*/
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * ImGui depends on the renderer
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


		virtual void registerObjectCreators(rtti::Factory& factory) override;

	private:
		RenderService*				mRenderer = nullptr;	///< The rendered used by IMGUI
	};
}