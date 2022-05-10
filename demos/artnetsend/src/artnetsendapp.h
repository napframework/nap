/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderwindow.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <scene.h>
#include <app.h>
#include <artnetcontroller.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that demonstrates the use of ArtNetController, which sends Art-Net.
	 * Use Napkin to change properties like the Net, SubNet and Universe for sending Art-Net.
	 */
	class ArtNetSendApp : public App
	{
		RTTI_ENABLE(App)
	public:
		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		ArtNetSendApp(nap::Core& core) : App(core) { }

		/**
		 * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
		*/
		bool init(utility::ErrorState& error) override;

		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

		/**
		 * Render is called after update. Use this call to render objects to a specific target
		 */
		void render() override;

		/**
		 * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;

		/**
		 * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;

		/**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
		 */
		virtual int shutdown() override;

	private:

		void showGeneralInfo();
		void showSendArtnet();

		ResourceManager*			mResourceManager = nullptr;		///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;		///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;		///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;		///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;			///< Manages GUI related update / draw calls
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;		///< Pointer to the render window
		ObjectPtr<Scene>			mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<ArtNetController>	mArtNetController = nullptr;	///< Pointer to the controller responsible for sending Art-Net
		int32_t						mArtNetOutputChannelCount = 16;	///< The amount of channels used for sending Art-Net
		std::vector<int32_t>		mArtNetOutputChannels;			///< The channel data for sending Art-Net
	};
}
