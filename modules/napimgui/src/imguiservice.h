/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "imgui/imgui.h"
#include "imguiicon.h"

// External includes
#include <nap/service.h>
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
	class Display;
	class GuiWindow;
	class IMGuiService;

	/**
	 * All available (default) icons: managed by the IMGuiService and guaranteed to exist.
	 * Use these names as identifier to look up a specific icon inside a module or application.
	 * 
	 * To consider: If this method of identification causes naming conflicts in the future,
	 * consider introducing explicit icon classes or coupling icons to modules instead.
	 * This will increase binary size but prevents name clashes if a module loads 
	 * icons with duplicate names, which currently is not allowed.
	 */
	namespace icon
	{
		inline constexpr const char* save		= "save.png";
		inline constexpr const char* saveAs		= "save_as.png";
		inline constexpr const char* cancel		= "cancel.png";
		inline constexpr const char* ok			= "ok.png";
		inline constexpr const char* del		= "delete.png";
		inline constexpr const char* file		= "file.png";
		inline constexpr const char* help		= "help.png";
		inline constexpr const char* settings	= "settings.png";
		inline constexpr const char* reload		= "reload.png";
		inline constexpr const char* folder		= "folder.png";
		inline constexpr const char* load		= "load.png";
		inline constexpr const char* info		= "info.png";
		inline constexpr const char* warning	= "warning.png";
		inline constexpr const char* error		= "error.png";
		inline constexpr const char* copy		= "copy.png";
		inline constexpr const char* paste		= "paste.png";
		inline constexpr const char* insert		= "insert.png";
		inline constexpr const char* edit		= "edit.png";
		inline constexpr const char* remove		= "remove.png";
		inline constexpr const char* add		= "add.png";
		inline constexpr const char* change		= "change.png";
	}


	// GUI
	namespace gui
	{
		inline constexpr float dpi = 96.0f;						///< Default (reference) dpi for gui elements

		/**
		 * All available color schemes
		 */
		enum class EColorScheme
		{
			Light		= 0,		///< Lighter color scheme
			Dark		= 1,		///< Darker color scheme (default)
			HyperDark	= 2,		///< High contrast dark color scheme
			Classic		= 3,		///< Classic color scheme
			Custom		= 4			///< Custom color scheme
		};

		/**
		 * Configurable palette of GUI colors.
		 */
		struct NAPAPI ColorPalette
		{
			ColorPalette() = default;
			RGBColor8 mBackgroundColor = { 0x2D, 0x2D, 0x2D };		///< Property: 'BackgroundColor' Gui window background color
			RGBColor8 mDarkColor = { 0x00, 0x00, 0x00 };			///< Property: 'DarkColor' Gui dark color
			RGBColor8 mMenuColor = { 0x8D, 0x8B, 0x84 };			///< Property: 'MenuColor' Gui menu color
			RGBColor8 mFront1Color = { 0x8D, 0x8B, 0x84 };			///< Property: 'FrontColor1' Gui gradient color 1
			RGBColor8 mFront2Color = { 0xAE, 0xAC, 0xA4 };			///< Property: 'FrontColor2' Gui gradient color 2
			RGBColor8 mFront3Color = { 0xCD, 0xCD, 0xC3 };			///< Property: 'FrontColor3' Gui gradient color 3
			RGBColor8 mFront4Color = { 0xFF, 0xFF, 0xFF };			///< Property: 'FrontColor4' Gui gradient color 4 (text)
			RGBColor8 mHighlightColor1 = { 0x29, 0x58, 0xff };		///< Property: 'HighlightColor1' Special highlight color 1 (selection)
			RGBColor8 mHighlightColor2 = { 0xD6, 0xFF, 0xA3 };		///< Property: 'HighlightColor2' Special highlight color 2 (info)
			RGBColor8 mHighlightColor3 = { 0xFF, 0xEA, 0x30 };		///< Property: 'HighlightColor3' Special highlight color 3 (warning)
			RGBColor8 mHighlightColor4 = { 0xFF, 0x50, 0x50 };		///< Property: 'HighlightColor4' Special highlight color 4 (errors)
			bool mInvertIcon = false;								///< Property: 'InvertIcon' If icons should be inverted
		};
	}


	/**
	 * GUI configuration options
	 */
	class NAPAPI IMGuiServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		gui::EColorScheme mColorScheme = gui::EColorScheme::Dark;		///< Property: 'Color Scheme' The color scheme to use (dark, light, custom etc.)
		float mFontSize = 17.0f;										///< Property: 'Font Size' Gui font size
		float mScale = 1.0f;											///< Property: 'Scale' Overall gui multiplication factor. Applies to the font and all other gui elements
		std::string mFontFile = "";										///< Property: 'FontFile' Path to a '.ttf' font file. If left empty the default NAP font will be used
		glm::ivec2 mFontOversampling = { 5, 3 };						///< Property: 'FontSampling' Horizontal and vertical font oversampling, higher values result in sharper text in exchange for more memory.
		float mFontSpacing = 0.25f;										///< Property: 'FontSpacing' Extra horizontal spacing (in pixels) between glyphs.
		gui::ColorPalette mCustomColors;								///< Property: 'Colors' Gui color overrides if scheme is set to custom
		virtual rtti::TypeInfo getServiceType() const override	{ return RTTI_OF(IMGuiService); }
	};
	 

	/**
	 * This service manages the global ImGui state.
	 * Call draw() inside a window render pass to draw the GUI to screen.
	 * Call selectWindow() to select the window subsequent ImGUI calls apply to.
	 * Explicit window selection is only necessary when there is more than 1 window.
	 * 
	 * Only call selectWindow() on application update, not when rendering the GUI to screen.
	 * The service automatically creates a new GUI frame before application update.
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
		 * Draws the GUI elements for the currently active window to screen.
		 * ~~~~~{.cpp}
		 *	// Draw gui window 1
		 *	mRenderService->beginFrame();
		 *	if (mRenderService->beginRecording(*mRenderWindowOne))
		 *	{
		 *		mRenderWindowOne->beginRendering();
		 *		mGuiService->draw();
		 *		mRenderWindowOne->endRendering();
		 *		mRenderService->endRecording();
		 *	}
		 * 
		 *	// Draw gui window 2
		 *	if (mRenderService->beginRecording(*mRenderWindowTwo))
		 *	{
		 *		mRenderWindowTwo->beginRendering();
		 *		mGuiService->draw();
		 *		mRenderWindowTwo->endRendering();
		 *		mRenderService->endRecording();
		 *	}
		 *	mRenderService->endFrame();
		 * ~~~~~
		 */
		void draw();

		/**
		 * Select the window all subsequent ImGUI calls apply to.
		 * Explicit selection is only necessary when there is more than 1 window.
		 * Only call selectWindow() on application update, not when rendering the GUI to screen.
		 *
		 * ~~~~~{.cpp}
		 *	mGuiService->selectWindow(mRenderWindowOne);
		 *	ImGui::Begin("GUI Window One");
		 *	...
		 *	ImGui::End();
		 *
		 *	mGuiService->selectWindow(mRenderWindowTwo);
		 *	ImGui::Begin("GUI Window Two");
		 *	...
		 *	ImGui::End();
		 * ~~~~~
		 * @param window the window to select
		 */
		void selectWindow(nap::ResourcePtr<RenderWindow> window);

		/**
		 * Returns the ImGUI context associated with the given window.
		 * @param window the render window
		 * @return ImGUI context for the given window, asserts if it doesn't exist.
		 */
		ImGuiContext* getContext(nap::ResourcePtr<RenderWindow> window);

		/**
		 * Returns the scaling factor of the current active context.
		 * The scaling factor is calculated using the display DPI (if high DPI rendering is enabled)
		 * and the global GUI scale.
		 * @return the scaling factor for the current active context, -1.0 if no context is active
		 */
		float getScale() const;

		/**
		 * Returns the scaling factor for the given context.
		 * The scaling factor is calculated using the display DPI (if high DPI rendering is enabled)
		 * and the global GUI scale.
		 * @param context the ImGUI context
		 * @return the scaling factor for the given context
		 */
		float getScale(const ImGuiContext* context) const;

		/**
		 * Returns the GUI color palette.
		 * @return the GUI color palette.
		 */
		const gui::ColorPalette& getPalette() const;

		/**
		 * Forwards window input events to the GUI, called from GUIAppEventHandler.
		 * @return context that belongs to the event, nullptr if the event is not related to a window.
		 */
		ImGuiContext* processInputEvent(InputEvent& event);

		/**
		 * @return if the GUI is capturing keyboard events
		 */
		bool isCapturingKeyboard(ImGuiContext* context);

		/**
		 * @return if the GUI is capturing mouse events
		 */
		bool isCapturingMouse(ImGuiContext* context);

		/**
		 * Returns a texture handle that can be used to display a Vulkan texture inside ImGUI.
		 * Alternatively, use the ImGUI::Image(nap::Texture2D&, ...) utility function, to immediately display a texture instead.		 
		 * Internally the handles are cached, it is therefore fine to call this function every frame. 
		 * Keep in mind that a handle (descriptor set) is created for every unique texture.
		 *
		 * ~~~~~{.cpp}
		 * ImGui::Begin("Texture");
		 * ImGui::Image(mGuiService.getTextureHandle(texture), ...);
		 * ImGui::End();
		 * ~~~~~
		 * @return Vulkan texture handle, used to display a texture in ImGUI
		 */
		ImTextureID getTextureHandle(const nap::Texture2D& texture) const;

		/**
		 * Returns an icon with the given name and extension.
		 * Note that the icon must exist. Only ask for icons with their 
		 * names explicitly declared in code, for example: 'icon::save'
		 * ~~~~~{.cpp}
		 *	if (ImGui::ImageButton(gui_service.getIcon(icon::ok)))
		 *	{
		 *		...
		 *	}
		 * ~~~~~
		 * @param name the name, including extension, of the icon to load
		 * @return icon with the given name
		 */
		nap::Icon& getIcon(std::string&& name);

		/**
		 * Create and add an icon to the default set of icons.
		 * The system looks for the icon in the data search paths of the module.
		 * On success call getIcon() to retrieve and draw the icon.
		 * 
		 * Call this function on initialization of your service and make sure your icon names are unique.
		 * Duplicates are not allowed!
		 * 
		 * ~~~~~{.cpp}
		 *	const auto& icon_names = icon::sequencer::get();
		 *	for (const auto& icon_name : icon_names)
		 *	{
		 *		if (!mGuiService->loadIcon(icon_name, this->getModule(), errorState))
		 *			return false;
		 *	}
		 * ~~~~~
		 * 
		 * @param name the name of the icon, including extension
		 * @param module the module that points to the icon
		 * @param error contains the error message if the load operations fails
         * @return if the icon is loaded and added to system defaults
         */
		bool loadIcon(const std::string& name, const nap::Module& module, utility::ErrorState& error);

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
		 * Saves all gui .ini files
		 */
		virtual void preShutdown() override;

		/**
		 *	Deletes all GUI related resources
		 */
		virtual void shutdown() override;

	protected:
		virtual void registerObjectCreators(rtti::Factory& factory) override;

	private:
		/**
		 * Simple struct that combines an ImGUI context with additional state information.
		 * Takes ownership of the context, destroys it on destruction.
		 */
		struct NAPAPI GUIContext
		{
			GUIContext(ImGuiContext* context, ImGuiStyle* style);
			~GUIContext();

			bool mMousePressed[3]			= { false, false, false };
			float mMouseWheel				= 0.0f;
			float mScale					= 1.0f;
			const Display* mDisplay			= nullptr;
			ImGuiContext* mContext			= nullptr;
			ImGuiContext* mPreviousContext	= nullptr;
			ImGuiStyle* mStyle				= nullptr;

			// Activates current context
			void activate();

			// Deactivates current context, restores previous
			void deactivate();
		};

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
		 * Called when a window event occurs.
		 */
		void onWindowEvent(const WindowEvent& windowEvent);
		nap::Slot<const WindowEvent&> mWindowEventSlot = { this, &IMGuiService::onWindowEvent };

		/**
		 * Creates all vulkan related resources, for imGUI as well as local
		 */
		void createVulkanResources(nap::RenderWindow& window);

		/**
		 * Starts a new imgui frame
		 */
		void newFrame(RenderWindow& window, GUIContext& context, double deltaTime);

		/**
		 * Calculates and applies a gui scaling factor based on the given display and associated dpi settings
		 */
		void pushScale(GUIContext& context, const Display& display);

		RenderService* mRenderService = nullptr;
		mutable std::unordered_map<const Texture2D*, VkDescriptorSet> mDescriptors;
		std::unique_ptr<DescriptorSetAllocator> mAllocator;
		std::unordered_map<RenderWindow*, std::unique_ptr<GUIContext>> mContexts;
		std::unique_ptr<ImFontAtlas> mFontAtlas = nullptr;
		std::unique_ptr<ImGuiStyle> mStyle = nullptr;
		IMGuiServiceConfiguration* mConfiguration = nullptr;
		float mGuiScale = 1.0f;		///< Overall GUI scaling factor
		float mDPIScale = 1.0f;		///< Max font scaling factor, based on the highest display dpi or 1.0 (default) when high dpi if off

		// Color palette
		const gui::ColorPalette* mColorPalette = nullptr;

		// Icons
		std::unordered_map<std::string, std::unique_ptr<Icon>> mIcons;
	};
}
