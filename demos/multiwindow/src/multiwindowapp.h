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
#include <imguiservice.h>
#include <app.h>
#include <imagefromfile.h>
#include <renderservice.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * This example displays 3 windows that draw 2 objects with different cameras and materials
	 * Input is handled separately for the first and third window. Window 2 is static and doesn't react to mouse events
	 * The most important part call of this app is the render call where we render the objects to each individual window
	 * This example uses 3 cameras, 2 perspective and 1 orthographic. The perspective cameras react to mouse input, similar
	 * to the helloworld example. The orthographic camera is used to draw the planes that show the NAP texture.
	 * Window 3 uses 2 cameras and renders in 2 passes. For each pass a different camera is used:
	 * first the sphere is rendered with a perspective camera, after that the transparent overlay texture with the orthographic camera.
	 * Only 2 meshes are used: a sphere and a plane. The plane receives the same material but a different texture for every window
	 * It's important to study the JSON file to relate the scene and it's objects to the render / update logic in this app.
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to 2 camera entities, associated with window 1 and 3
	 */
	class MultiWindowApp : public App
	{
		RTTI_ENABLE(App)
	public:
		MultiWindowApp(nap::Core& core) : App(core)	{ }

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

		/**
		 *	Positions the texture plane in the center of the window based on the window resolution
		 */
		void positionPlane(nap::RenderWindow& window, nap::TransformComponentInstance& planeTransform);

		/**
		 * Updates the GUI for all windows
		 */
		void updateGUI();

	private:
		// Nap Services
		RenderService*				mRenderService = nullptr;			//< Render Service that handles render calls
		ResourceManager*			mResourceManager = nullptr;			//< Manages all the loaded resources
		SceneService*				mSceneService = nullptr;			//< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			//< Input service for processing input

		ObjectPtr<RenderWindow>		mRenderWindowOne;					//< First window
		ObjectPtr<RenderWindow>		mRenderWindowTwo;					//< Second window
		ObjectPtr<RenderWindow>		mRenderWindowThree;					//< Third window

		ObjectPtr<EntityInstance>	mPerspectiveCameraOne = nullptr;	//< Pointer to the entity that holds the cam for window 1
		ObjectPtr<EntityInstance>	mPerspectiveCameraTwo = nullptr;	//< Pointer to the entity that holds the cam for window 3
		ObjectPtr<EntityInstance>	mOrthoCamera = nullptr;				//< Pointer to the entity that holds the orthographic camera for window 2
		ObjectPtr<EntityInstance>	mWorldEntity = nullptr;				//< Pointer to the entity that holds the world
		ObjectPtr<EntityInstance>	mPlaneOneEntity = nullptr;			//< Pointer to the plane one
		ObjectPtr<EntityInstance>	mPlaneTwoEntity = nullptr;			//< Pointer to plane two
		ObjectPtr<ImageFromFile>	mTextureOne = nullptr;				//< Pointer to the first texture
		ObjectPtr<ImageFromFile>	mTextureTwo = nullptr;				//< Pointer to the second texture
		ObjectPtr<ImageFromFile>	mWorldTexture = nullptr;			//< Pointer to the world texture

		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls

		RGBColorFloat mColorOne;										//< First sphere blend color
		RGBColorFloat mColorTwo;										//< Second sphere blend color
		RGBColorFloat mHaloColor;										//< Sphere halo color
	};
}
