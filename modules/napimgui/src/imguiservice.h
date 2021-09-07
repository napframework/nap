/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	// Reference dpi
	inline constexpr float referenceDPI = 96.0f;

	// Forward Declares
	class RenderService;
	class Display;
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
		float mScale				= 1.0f;								///< Property: 'Scale' Overall gui multiplication factor. Applies to the font and all other gui elements
		std::string mFontFile 		= "";								///< Property: 'FontFile' Path to a '.ttf' font file in the data folder. If left empty the default NAP font will be used
		RGBColor8 mHighlightColor	= RGBColor8(0xC8, 0x69, 0x69);		///< Property: 'HighlightColor' Gui highlight color
		RGBColor8 mBackgroundColor	= RGBColor8(0x2D, 0x2E, 0x42);		///< Property: 'BackgroundColor' Gui background color
		RGBColor8 mDarkColor		= RGBColor8(0x11, 0x14, 0x26);		///< Property: 'DarkColor' Gui dark color
		RGBColor8 mFront1Color		= RGBColor8(0x52, 0x54, 0x6A);		///< Property: 'FrontColor1' Gui front color 1
		RGBColor8 mFront2Color		= RGBColor8(0x5D, 0x5E, 0x73);		///< Property: 'FrontColor2' Gui front color 2
		RGBColor8 mFront3Color		= RGBColor8(0x8B, 0x8C, 0xA0);		///< Property: 'FrontColor3' Gui front color 3
		virtual rtti::TypeInfo getServiceType() const override			{ return RTTI_OF(IMGuiService); }
	};

	/**
	 * This service manages the global ImGui state.
	 * Call draw() inside a window render pass to draw the GUI to screen.
	 * Call selectWindow() to select the window subsequent ImGUI calls apply to.
	 * Explicit window selection is only necessary when there is more than 1 window.
	 * 
	 * Only call selectWindow() on application update, not when rendering the GUI to screen.
	 * The service automatically creates a new GUI frame before application update.
	 *
	 * MyApp::update(double deltaTime):
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
	 *
	 * MyApp::render()
	 * ~~~~~{.cpp}
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
		 * @return ImGUI context for the given window, asserts if it doesn't exist.
		 */
		ImGuiContext* getContext(nap::ResourcePtr<RenderWindow> window);

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
		 * Simple struct that combines an ImGUI context with additional state information.
		 * Takes ownership of the context, destroys it on destruction.
		 */
		struct NAPAPI GUIContext
		{
			GUIContext(ImGuiContext* context, ImGuiStyle* style);
			~GUIContext();

			bool mMousePressed[3]			= { false, false, false };
			float mMouseWheel				= 0.0f;
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
		std::unordered_map<Texture2D*, VkDescriptorSet> mDescriptors;
		std::unique_ptr<DescriptorSetAllocator> mAllocator;
		std::unordered_map<RenderWindow*, std::unique_ptr<GUIContext>> mContexts;
		std::unique_ptr<ImFontAtlas> mFontAtlas = nullptr;
		std::unique_ptr<ImGuiStyle> mStyle = nullptr;
		IMGuiServiceConfiguration* mConfiguration = nullptr;
		float mGuiScale = 1.0f;		///< Overall GUI scaling factor
		float mDPIScale = 1.0f;		///< Max font scaling factor, based on the highest display dpi or 1.0 (default) when high dpi if off
	};
}
