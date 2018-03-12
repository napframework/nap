#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <app.h>
#include <scene.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class @PROJECT_NAME_CAMELCASE@App : public App
	{
		RTTI_ENABLE(App)
	public:
		@PROJECT_NAME_CAMELCASE@App(nap::Core& core) : App(core) { }
		
		/**
		 *	Initialize all the services and app specific data structures
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
		 *	Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);

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
		int shutdown() override;
		
		
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
		
		std::vector<ResourcePtr<RenderWindow>> mRenderWindows;		//< Vector holding pointers to the spawned render windows
		
		ResourcePtr<EntityInstance> mCameraEntity;					//< The entity that holds the camera
		
		ResourcePtr<EntityInstance> mRootLayoutEntity;				//< Entity at the root of the layout
		ResourcePtr<EntityInstance> mSlideShowEntity;					//< The slideshow entity
		ResourcePtr<EntityInstance> mUiInputRouter;					//< Our UI input router entity

		ResourcePtr<Scene>		mScene;								//< Scene in json file
	};
}
