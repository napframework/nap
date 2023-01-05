/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "copystampapp.h"

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop.
	 * Note that this demo does not run well in Debug mode!
	 *
	 * This demo application shows you how to write a custom nap::RenderableComponent.
	 * The component in this demo (nap::RenderableCopyMeshComponent) copies 3D meshes onto the vertices of a reference mesh.
	 * The copy / stamping is handled at render-time, making sure no additional CPU / memory resources are needed.
	 *
	 * Change the number of copies by changing the amount of rows / columns in the StampPlane resource (json).
	 * You can also add more meshes that are copied by editing the CopyMeshes array of the nap::RenderableCopyMeshComponent (json).
	 * Take a look at the custom nap::RenderableCopyMeshComponent to see how the copy stamping at render-time is actually done.
	 */
	class CopystampApp : public App
	{
		RTTI_ENABLE(App)
	public:
		CopystampApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
				
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService*				mRenderService = nullptr;			//< Render Service that handles render calls
		ResourceManager*			mResourceManager = nullptr;			//< Manages all the loaded resources
		SceneService*				mSceneService = nullptr;			//< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			//< Input service for processing input
		IMGuiService*				mGuiService = nullptr;				//< Gui service

		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			//< Pointer to the render window

		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;		//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance>	mWorldEntity = nullptr;			//< Pointer to the entity that holds the points that are copied onto

		// Updates gui components
		void updateGui();

		// Handle debug popup
		void handlePopup();
		bool mPopupOpened = false;
	};
}
