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

namespace nap 
{
	using namespace rtti;

    /**
     * Example application, called from within the main loop.
	 * 
	 * Use this app as a template for other apps that are created directly in 'source'.
	 * This example links to and uses it's own custom module: 'napexample'.
	 * More information and documentation can be found at: https://www.napframework.com/doxygen/
     */
    class CoreApp : public App 
	{
    public:
		/**
		 * Constructor
		 */
        CoreApp(nap::Core& core) : App(core) {}

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
        ResourceManager*			mResourceManager = nullptr;		///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;		///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;		///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;		///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;			///< Manages GUI related update / draw calls
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;		///< Pointer to the render window	
		ObjectPtr<Scene>			mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;		///< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mGnomonEntity = nullptr;		///< Pointer to the entity that can render the gnomon
		ObjectPtr<EntityInstance>	mTorusEntity = nullptr;			///< Pointer to the torus entity
		ObjectPtr<EntityInstance>	mPlaneEntity = nullptr;			///< Pointer to the plane entity

		// Render masks
		ObjectPtr<RenderTag>		mObjectTag = nullptr;			///< Pointer to the render object tag 
		ObjectPtr<RenderTag>		mDebugTag = nullptr;			///< Pointer to the render debug tag
	};
}
