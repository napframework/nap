/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <app.h>
#include <spheremesh.h>
#include <font.h>
#include <imagefromfile.h>
#include <snapshot.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a rotating textured sphere in the center of the viewport
	 * You can use the left mouse button to orbit around the object and 
	 * the right mouse button to zoom in on the object
	 * 
	 * This demo uses 3 important modules: mod_naprender, mod_napinput, mod_napcameracontrol
	 * The sphere is rendered using a simple material that blends 2 colors based on a texture's alpha value.
	 * The sphere is positioned by a transform component and rotated along the y axis by a rotate component.
	 * The camera is placed away from the origin but can be moved using the mouse. The camera looks at
	 * the sphere and uses that information to orbit around the object.
	 * Text is rendered in the middle of the viewport, on top of the sphere, using a nap::Renderable2DTextComponent.
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
	 */
	class HelloWorldApp : public App
	{
		RTTI_ENABLE(App)
	public:
		HelloWorldApp(nap::Core& core) : App(core)	{ }

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
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		ObjectPtr<EntityInstance> mTextEntity = nullptr;				//< Pointer to the entity that can display text
		ObjectPtr<EntityInstance> mWorldEntity = nullptr;				//< Pointer to the entity that holds the sphere
		ObjectPtr<EntityInstance> mPerspectiveCamEntity = nullptr;		//< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance> mOrthographicCamEntity = nullptr;		//< Pointer to the entity with an orthographic camera
		ObjectPtr<ImageFromFile> mWorldTexture = nullptr;				//< Pointer to the world texture
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color

		bool mTakeSnapshot = false;
		ObjectPtr<Snapshot> mSnapshot;
	};
}
