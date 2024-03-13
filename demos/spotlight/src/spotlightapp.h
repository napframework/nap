#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <app.h>
#include <rendertag.h>
#include <renderadvancedservice.h>

namespace nap 
{
	using namespace rtti;

	/**
	 * Demo application that shows how to setup and use a spotlight, including shadows, to light a minimal scene.
	 *
	 * The RenderAdvanced service call `renderShadows` is responsible for most of the
	 * heavy lifting as it updates the shadow maps of all lights and updates the uniform and sampler information of all
	 * compatible materials. When rendering our scene to the window using `RenderService::renderObjects`, all of the information is in
	 * place for the system to resolve and execute the render passes appropriately.
	 */
    class SpotlightApp : public App 
	{
    public:
		/**
		 * Constructor
		 */
		SpotlightApp(nap::Core& core) : App(core) {}

        /**
         * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
         */
        bool init(utility::ErrorState& error) override;

		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

        /**
         * Render is called after update. Use this call to render objects to a specific target
         */
        void render() override;

        /**
         * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
         */
        void windowMessageReceived(WindowEventPtr windowEvent) override;

        /**
         * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
         */
        void inputMessageReceived(InputEventPtr inputEvent) override;

        /**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
         */
        int shutdown() override;

    private:
        ResourceManager*			mResourceManager = nullptr;			///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;			///< Render Service that handles render calls
		RenderAdvancedService*		mRenderAdvancedService = nullptr;	///< Advanced Render Service that handles lights and shadows
		SceneService*				mSceneService = nullptr;			///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;				///< Manages GUI related update / draw calls
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			///< Pointer to the render window	
		ObjectPtr<Scene>			mScene = nullptr;					///< Pointer to the main scene
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;			///< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mSceneEntity = nullptr;				///< Pointer to the entity that holds the objects to render
		ObjectPtr<EntityInstance>	mSpotlightEntity = nullptr;			///< Pointer to the spotlight entity

		// Render masks
		ObjectPtr<RenderTag>		mSceneTag = nullptr;				///< Pointer to the render object tag

		// GUI states
		bool mShowLocators = false;										///< If light origin is shown
		bool mShowFrustrum = true;										///< If light frustrum is shown
	};
}
