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
		/**
		 *  Cycle-right button clicked
		 */
		void rightButtonClicked(const PointerPressEvent& evt);
		
		/**
		 *  Cycle-left button clicked
		 */
		void leftButtonClicked(const PointerPressEvent& evt);
		
		
		// Nap Services
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;				//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		
		InputService* mInputService = nullptr;						//< Input service for processing input
		
		std::vector<ObjectPtr<RenderWindow>> mRenderWindows;		//< Vector holding pointers to the spawned render windows
		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< The entity that holds the camera
		
		ObjectPtr<EntityInstance> mRootLayoutEntity = nullptr;		//< Entity at the root of the layout
		ObjectPtr<EntityInstance> mSlideShowEntity = nullptr;		//< The slideshow entity
		ObjectPtr<EntityInstance> mUiInputRouter = nullptr;			//< Our UI input router entity
	};
}
