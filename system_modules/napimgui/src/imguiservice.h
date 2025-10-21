/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "imguiicon.h"
#include "imguistyle.h"

// External includes
#include <nap/service.h>
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
        inline constexpr const char* subtract	= "subtract.png";
		inline constexpr const char* frame		= "frame.png";
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
		gui::ColorPalette mCustomColors;								///< Property: 'Colors' Color overrides if scheme is set to custom
		gui::Style mStyle;												///< Property: 'Style' Configurable style elements
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
		 * Returns the ImGUI context associated with the given window id.
		 * @param windowID the render window id
		 * @return ImGUI context for the given window, nullptr if it doesn't exist.
		 */
		ImGuiContext* findContext(int windowID);

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
		 * Sets the GUI color palette
		 * @param palette the color palette to apply 
		 */
		void setPalette(gui::EColorScheme palette);

		/**
		 * Forwards window input events to the GUI, called from GUIAppEventHandler.
		 * @return context that belongs to the event, nullptr if the event is not related to a window.
		 */
		ImGuiContext* processInputEvent(const InputEvent& event);

		/**
		 * @return if the GUI is capturing keyboard events
		 */
		bool isCapturingKeyboard(ImGuiContext* context);

		/**
		 * @return if the GUI is capturing mouse events
		 */
		bool isCapturingMouse(ImGuiContext* context);

		/**
		 * Add unicode input character.
		 * @param context the gui context
		 * @param character the unicode character to add
		 */
		void addInputCharachter(ImGuiContext* context, nap::uint character);

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
		bool init(utility::ErrorState& error) override;

		/**
		 * ImGui depends on the renderer
		 * @param dependencies the type of services this service depends on
		 */
		void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Allows IMGUI to draw a new frame. This is called automatically by core in the app loop
		 * @param deltaTime the time in seconds between ticks
		 */
		void update(double deltaTime) override;

		/**
		 * Ends frame operation for all contexts.
		 */
		void postUpdate(double deltaTime) override;

		/**
		 * Saves all gui .ini files
		 */
		void preShutdown() override;

		/**
		 *	Deletes all GUI related resources
		 */
		void shutdown() override;

		/**
		 * Registers all gui object creation objects
		 */
		void registerObjectCreators(rtti::Factory& factory) override;

	private:
		/**
		 * Simple struct that combines an ImGUI context with additional state information.
		 * Takes ownership of the context, destroys it on destruction.
		 */
		struct NAPAPI GUIContext
		{
			GUIContext(ImGuiContext* context, ImGuiStyle* style);
			~GUIContext();

			using ESource = PointerEvent::ESource;
			static constexpr int keyCount = 512;											///< Number of items in key array
			using KeyArray = std::array<bool, keyCount>;

			std::array<bool,3> mMousePressed				= { false };					///< If the mouse was pressed this frame
			std::array<bool,3> mMouseRelease				= { false };					///< If the mouse was released this frame
			std::array<bool,3> mModPressed					= { false };					///< If the ctrl (0), alt (1) or shift (2) modifier key is pressed
			std::array<bool,3> mModRelease					= { false };					///< If the ctrl (0), alt (1) or shift (2) modifier key is released
			std::array<int, 3> mPointerID					= { gui::pointerInvalidID };	///< Pointer id, -1 for pointer, 0+ for finger
			KeyArray mKeyPressed							= { false };					///< The keys that were pressed this frame
			std::vector<int> mKeyRelease;													///< The keys that were released this frame
			glm::ivec2 mMousePosition						= { 0, 0 };						///< Last known mouse position
			float mMouseWheel								= 0.0f;							///< Mouse wheel
			float mScale									= 1.0f;							///< GUI Scale
			int mDisplayIndex								= -1;							///< Current display
			ImGuiContext* mContext							= nullptr;						///< Associated ImGUI context
			ImGuiContext* mPreviousContext					= nullptr;						///< Context active before this one
			ImGuiStyle* mStyle								= nullptr;						///< Style of context

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
		 * Calculates and applies a gui scaling factor based for the given window
		 */
		void pushScale(GUIContext& context, const nap::RenderWindow& window);

		/**
		 * Add key event to given context
		 */
		void handleKeyEvent(const KeyEvent& keyEvent, GUIContext& context);

		/**
		 * Add pointer event to given context
		 */
		void handlePointerEvent(const PointerEvent& pointerEvent, GUIContext& context);

		/**
		 * Add touch event to given context
		 */
		void handleTouchEvent(const TouchEvent& touchEvent, GUIContext& context);

		// Everything related to ImGUI
		RenderService* mRenderService = nullptr;
		std::unordered_map<RenderWindow*, std::unique_ptr<GUIContext>> mContexts;
		std::unique_ptr<ImFontAtlas> mFontAtlas = nullptr;
		std::unique_ptr<ImGuiStyle> mStyle = nullptr;
		float mGuiScale = 1.0f;			///< Overall GUI scaling factor
		float mReferenceScale = 1.0f;		///< Max font scaling factor, based on the highest display dpi or 1.0 (default) when high dpi if off

		// Selected colour palette and style
		IMGuiServiceConfiguration* mConfiguration = nullptr;
		const gui::ColorPalette* mColorPalette = nullptr;

		// Icons
		std::unordered_map<std::string, std::unique_ptr<Icon>> mIcons;

		// Vulkan data associated with ImGUI
		std::unique_ptr<DescriptorSetAllocator> mAllocator;								///< GUI Descriptor set allocator
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;								///< GUI Pool used by font texture
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;					///< GUI texture layout description 
		VkSampler mSampler = VK_NULL_HANDLE;											///< GUI Texture binding
		mutable std::unordered_map<const Texture2D*, VkDescriptorSet> mDescriptors;		///< Cached GUI texture descriptors
	};
}
