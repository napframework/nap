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
#include <rendertarget.h>
#include <app.h>


namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class RenderTestApp : public App
	{
		RTTI_ENABLE(App)
	public:
		RenderTestApp(nap::Core& core) : App(core)				{ }
		
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
		void shutdown() override;
		
	private:
		
		// Nap Services
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;	//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		
		InputService* mInputService = nullptr;						//< Input service for processing input
		
		std::vector<ObjectPtr<RenderWindow>> mRenderWindows;		//< Vector holding pointers to the spawned render windows

		ObjectPtr<EntityInstance> mDefaultInputRouter = nullptr;	//< Default input router entity for processing input
		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< Entity that holds the camera
		ObjectPtr<EntityInstance> mCameraEntityLeft = nullptr;		//< Camera entity for first/left window
		ObjectPtr<EntityInstance> mCameraEntityRight = nullptr;		//< Camera entity for second/right window
		ObjectPtr<EntityInstance> mSplitCameraEntity = nullptr;		//< Split camera entity for world globe moving between windows
		
		ObjectPtr<RenderTarget> mTextureRenderTarget;				//< Render target for first window rotating plane
		ObjectPtr<EntityInstance> mPigEntity = nullptr;				//< Pig entity
		ObjectPtr<EntityInstance> mRotatingPlaneEntity = nullptr;	//< Rotating render target entity
		ObjectPtr<EntityInstance> mPlaneEntity = nullptr;			//< Warping custom line entity
		ObjectPtr<EntityInstance> mWorldEntity = nullptr;			//< World globe entity
	};
}
