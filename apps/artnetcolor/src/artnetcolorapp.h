#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <sdlinput.h>
#include <sdlwindow.h>
#include <nap/signalslot.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <artnetservice.h>
#include <artnetcontroller.h>
#include <app.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class ArtnetColorApp : public App
	{
		RTTI_ENABLE(App)
	public:
		ArtnetColorApp(nap::Core& core) : App(core)	{ }

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
		 *	Toggles full screen
		 */
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);
		
		/**
		 *	Called when loop finishes
		 */
		void shutdown() override;
		

	private:
		
		// Nap Services
		RenderService*		mRenderService = nullptr;				//< Render Service that handles render calls
		ResourceManager*	mResourceManager = nullptr;				//< Manages all the loaded resources
		SceneService*		mSceneService = nullptr;				//< Manages all the objects in the scene
		InputService*		mInputService = nullptr;				//< Input service for processing input
		ArtNetService*		mArtnetService = nullptr;				//< Manages ArtNET communication

		ObjectPtr<RenderWindow> mRenderWindow;						//< Vector holding pointers to the spawned render windows
		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance> mPlaneEntity = nullptr;			//< Pointer to the entity that holds the plane
		ObjectPtr<ArtNetController> mArtnetController = nullptr;	//< The art-net controller

		// Window event handling
		void handleWindowEvent(const WindowEvent& windowEvent);
		NSLOT(mWindowEventSlot, const nap::WindowEvent&, handleWindowEvent)
	};
}
