#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <app.h>
#include <nap/core.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class SteefApp : public App
	{
		RTTI_ENABLE(App)
	public:
		SteefApp(nap::Core& core) : nap::App(core)								{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		virtual bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		virtual void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		virtual int shutdown() override;

		/**
		*	Toggles full screen
		*/
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);

		/**
		*	Called when a window event is received
		*/
		void handleWindowEvent(const WindowEvent& windowEvent);
		
	private:
		
		void updateBackgroundImage();
		void updateShader();
		
		ObjectPtr<ImageFromFile> mVinylLabelImg = nullptr;						//< Vinyl Label Image Resource
		ObjectPtr<ImageFromFile> mVinylCoverImg = nullptr;						//< Vinyl Cover Image Resource
		
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
