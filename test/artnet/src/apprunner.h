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
#include <inputrouter.h>
#include <artnetservice.h>
#include "artnetcontroller.h"

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class AppRunner
	{
		
	public:
		AppRunner();
		~AppRunner() = default;
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(Core& core);
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update();

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render();

		/**
		 *	Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);

		/**
		 *	Forwards the received window event to the render service
		 */
		void registerWindowEvent(WindowEventPtr windowEvent);
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void registerInputEvent(InputEventPtr inputEvent);
		
		/**
		 *	Toggles full screen
		 */
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);
		
		/**
		 *	Called when loop finishes
		 */
		void shutdown();
		
	private:
		
		// Nap Services
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManagerService* mResourceManagerService = nullptr;	//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		
		InputService* mInputService = nullptr;						//< Input service for processing input
		ArtNetService* mArtnetService = nullptr;					//< Manages ArtNET communication
		
		std::vector<ObjectPtr<RenderWindow>> mRenderWindows;		//< Vector holding pointers to the spawned render windows
		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< Pointer to the entity that holds the camera
	};
}
