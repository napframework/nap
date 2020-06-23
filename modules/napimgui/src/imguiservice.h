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
#include <color.h>

// ImGUI forward declares
struct ImGuiContext;

namespace nap
{
	// Forward Declares
	class RenderService;
	class GuiWindow;
	class IMGuiService;

	/**
	 * IMGUI configuration options
	 */
	class NAPAPI IMGuiServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)

	public:
		float mFontSize				= 17.0f;							///< Property: 'FontSize' Gui Font Size
		RGBColor8 mHighlightColor	= RGBColor8(0xC8, 0x69, 0x69);		///< Property: 'HighlightColor' Gui highlight color
		RGBColor8 mBackgroundColor	= RGBColor8(0x2D, 0x2E, 0x42);		///< Property: 'BackgroundColor' Gui background color
		RGBColor8 mDarkColor		= RGBColor8(0x11, 0x14, 0x26);		///< Property: 'DarkColor' Gui dark color
		RGBColor8 mFront1Color		= RGBColor8(0x52, 0x54, 0x6A);		///< Property: 'FrontColor1' Gui front color 1
		RGBColor8 mFront2Color		= RGBColor8(0x5D, 0x5E, 0x73);		///< Property: 'FrontColor2' Gui front color 2
		RGBColor8 mFront3Color		= RGBColor8(0x8B, 0x8C, 0xA0);		///< Property: 'FrontColor3' Gui front color 3
		virtual rtti::TypeInfo		getServiceType() override			{ return RTTI_OF(IMGuiService); }
	};

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
		void draw();

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
		 * Ends frame operation for all contexts.
		 */
		virtual void postUpdate(double deltaTime) override;

		/**
		 *	Deletes all GUI related resources
		 */
		virtual void shutdown() override;

	private:

		/**
		 * Simple struct that combines an ImGUI context with additional state information
		 * Takes ownership of the context, destroys it on destruction
		 */
		struct GUIContext
		{
			GUIContext(ImGuiContext* context) : mContext(context) { };
			~GUIContext();

			bool mMousePressed[3]		= { false, false, false };
			float mMouseWheel			= 0.0f;
			ImGuiContext* mContext		= nullptr;
		};

		RenderService* mRenderService = nullptr;
		std::unordered_map<Texture2D*, VkDescriptorSet> mDescriptors;
		std::unique_ptr<DescriptorSetAllocator> mAllocator;
		std::unordered_map<RenderWindow*, std::unique_ptr<GUIContext>> mContexts;
		std::unique_ptr<ImFontAtlas> mFontAtlas = nullptr;
		VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;

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
		 * Creates all vulkan related resources, for imGUI as well as local
		 */
		void createVulkanResources(nap::RenderWindow& window);

		/**
		 * starts a new imgui frame
		 */
		void newFrame(RenderWindow& window, GUIContext& context, double deltaTime);
	};
}