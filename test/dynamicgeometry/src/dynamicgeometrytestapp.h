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
	class DynamicGeometryTestApp : public App
	{
		RTTI_ENABLE(App)
	public:
		DynamicGeometryTestApp(nap::Core& core) : App(core)				{ }
		
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
		 *	Called when loop finishes
		 */
		void shutdown() override;
		
	private:
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;				//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		InputService* mInputService = nullptr;						//< Input service for processing input
		ObjectPtr<RenderWindow> mRenderWindow;						//< Pointers to the render window
		ObjectPtr<EntityInstance> mDefaultInputRouter;				///< 
		ObjectPtr<EntityInstance> mCameraEntity;					//< Entity that holds the camera
		ObjectPtr<RenderTarget> mTextureRenderTarget;				//< Render target for first window rotating plane
	};
}
