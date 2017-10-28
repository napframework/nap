#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <etherdreamservice.h>
#include <oscservice.h>
#include <app.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class EtherdreamApp : public App
	{
		RTTI_ENABLE(App)
	public:
		EtherdreamApp(nap::Core& core) : App(core)		{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& error) override;

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
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;	//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		InputService* mInputService = nullptr;						//< Input service for processing input
		EtherDreamService* mLaserService = nullptr;					// < Laser service
		OSCService* mOscService = nullptr;							// < Laser DAC
	
		ObjectPtr<RenderWindow> mRenderWindow = nullptr;			//< Pointers to the render window// Laser DAC
		ObjectPtr<EntityInstance> mLaserController = nullptr;		//< Entity that holds all the lasers to update / draw
		ObjectPtr<EntityInstance> mLaserCamera = nullptr;			//< Entity that holds the camera that is used to render the laser to a backbuffer
		ObjectPtr<EntityInstance> mFrameCamera = nullptr;			//< Entity that holds the camera that is used to render all the backbuffers to screen
	};
}
