/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <lookingglassservice.h>
#include <parameter.h>
#include <parametergui.h>
#include <app.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that demonstrates real-time rendering to the Looking Glass display using `mod_naplookingglass`.
	 * 
	 * Use the left mouse button to orbit around the object and the right mouse button to zoom in on the focus position.
	 * Enabling `AutoFocus` lets you control the camera size as opposed to its location. The distance to the focus
	 * position is sustained by the `nap::FocusController`, keeping the objects in focus on the Looking Glass.
	 *
	 * Several different objects are rendered in this demo:
	 * - A simple box wireframe.
	 * - A regular renderable mesh component with a mesh loaded from disk.
	 * - A line of 3D text.
	 * - A particle system that uses instanced rendering.
	 *
	 * The visibility of each can be toggled in the control GUI window. Finally, a small set of parameters enable the
	 * control over several visual aspects of the particle system.
	 *
	 * The NAP x Looking Glass rendering workflow really only requires rendering your scene to a `nap::QuiltRenderTarget`
	 * and then rendering `nap::RenderLightFieldComponentInstance` with the resulting quilt to a `nap::RenderWindow`
	 * displayed on the Looking Glass, as shown in this demo.
	 */
	class HoloApp : public App
	{
		RTTI_ENABLE(App)
	public:
		HoloApp(nap::Core& core) : App(core)	{ }

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
		RenderService*				mRenderService = nullptr;							//< Render Service that handles render calls
		ResourceManager*			mResourceManager = nullptr;							//< Manages all the loaded resources
		SceneService*				mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService*				mInputService = nullptr;							//< Input service for processing input
		IMGuiService*				mGuiService = nullptr;								//< Gui service
		LookingGlassService*		mLookingGlassService = nullptr;						//< Looking glass service

		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;							//< Pointer to the render window
		ObjectPtr<RenderWindow>		mLookingGlassRenderWindow = nullptr;				//< Pointer to the render window

		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;							//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance>	mWorldEntity = nullptr;								//< Pointer to the entity that holds the world
		ObjectPtr<EntityInstance>	mObjectEntity = nullptr;							//< Pointer to the entity that holds the object

		ObjectPtr<EntityInstance>	mWindowTextureEntity = nullptr;						//< Pointer to the entity that holds the texture to render to the preview window
		ObjectPtr<EntityInstance>	mLookingGlassWindowTextureEntity = nullptr;			//< Pointer to the entity that holds the texture to render to the looking glass

		ObjectPtr<ParameterGroup>	mParameterGroup = nullptr;							//< Parameters
		ObjectPtr<ParameterGUI>		mParameterGUI = nullptr;							//< Parameter GUI to display

		RGBAColor8					mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
	};
}
