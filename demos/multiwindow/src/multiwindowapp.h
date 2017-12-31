#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>

namespace nap
{
	/**
	 * Demo application that is called from within the main loop
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
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
		void shutdown() override;

		/**
		 *	Positions the texture plane in the center of the window based on the window resolution
		 */
		void positionPlane(nap::RenderWindow& window, nap::TransformComponentInstance& planeTransform);

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
	};
}
