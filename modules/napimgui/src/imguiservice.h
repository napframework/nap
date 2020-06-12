#pragma once

// Local Includes
#include "imgui/imgui.h"

// External includes
#include <nap/service.h>
#include <utility/dllexport.h>
#include <renderwindow.h>
#include <inputevent.h>
#include <nap/resourceptr.h>
#include <descriptorsetallocator.h>
#include <nap/signalslot.h>

// ImGUI forward declares
struct ImGuiContext;

namespace nap
{
	// Forward Declares
	class RenderService;
	class GuiWindow;
	//class DescriptorSetAllocator;

	/**
	 * This service manages the global ImGui state.
	 * Use selectWindow() to select the window to draw the GUI on to.
	 * By default the GUI is drawn to the primary window, as defined by the renderer.
	 * Make sure to call draw() inside your application to render the gui to the right window. 
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
		void draw(VkCommandBuffer commandBuffer, nap::RenderWindow& window);

		/**
		 * Explicitly set the window that is used for drawing the GUI elements
		 * When no window is specified the system uses the primary window to draw GUI elements
		 * Only set the window on init() of your application.
		 * @param window the window to use for drawing the GUI elements
		 */
		void selectWindow(nap::ResourcePtr<RenderWindow> window);

		/**
		 * Handles input for gui related tasks, called from the Gui App Event Handler
		 * This is separate from other input related event handling
		 */
		void processInputEvent(InputEvent& event);

		/**
		 * @return if the gui is capturing keyboard events
		 */
		bool isCapturingKeyboard();

		/**
		 * @return if the gui is capturing mouse events
		 */
		bool isCapturingMouse();

		/**
		 * @return Vulkan texture handle, can be used to display a texture in ImGUI
		 */
		ImTextureID getTextureHandle(nap::Texture2D& texture);

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
		RenderService*				mRenderService = nullptr;		///< The rendered used by IMGUI
		std::unordered_map<Texture2D*, VkDescriptorSet> mDescriptors;
		std::unique_ptr<DescriptorSetAllocator> mAllocator;
		std::unordered_map<RenderWindow*, ImGuiContext*> mContexts;
		std::unique_ptr<ImFontAtlas> mFontAtlas = nullptr;

		/**
		 * Called when a window is added, creates ImGUI related resources
		 */
		void onWindowAdded(RenderWindow& window);
		nap::Slot<RenderWindow&> mWindowAddedSlot	= { this, &IMGuiService::onWindowAdded };

		/**
		 * Called when a window is removed, destroys ImGUI related resources
		 */
		void onWindowRemoved(RenderWindow& window);
		nap::Slot<RenderWindow&> mWindowRemovedSlot = { this, &IMGuiService::onWindowRemoved };

		/**
		 * Creates a new imgui context
		 */
		ImGuiContext* createContext(ImFontAtlas& fontAtlas);

		/**
		 * Creates all vulkan related resources, for imGUI as well as local
		 */
		void createVulkanResources(nap::RenderWindow& window);
	};
}