#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>

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
		bool init(Core& core, utility::ErrorState& error);
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime);

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
		
		void updateBackgroundImage();
		void updateShader();
		
		ObjectPtr<Image> mVinylLabelImg = nullptr;						//< Vinyl Label Image Resource
		ObjectPtr<Image> mVinylCoverImg = nullptr;						//< Vinyl Cover Image Resource
		
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager*	mResourceManager = nullptr;		//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		
		InputService* mInputService = nullptr;							//< Input service for processing input
		
		ObjectPtr<RenderWindow>	mRenderWindow = nullptr;				//< Pointer to the spawned render window
		ObjectPtr<EntityInstance> mModelEntity = nullptr;				//< Pointer to the entity that holds all the vinyl parts
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;				//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance> mBackgroundEntity = nullptr;			//< Pointer to the entity that holds the background image
	};
}
