#pragma once

// Local Includes
#include "flexblockgui.h"

// Nap includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>

namespace nap
{
	using namespace rtti;

	/**
	 *	Atmos visualization application
	 */
	class FlexblockApp : public App
	{
		RTTI_ENABLE(App)
		friend class FlexblockGui;
	public:
		FlexblockApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
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
		int shutdown() override;

	private:
		// Nap Services
		RenderService*		mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager*	mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService*		mSceneService = nullptr;					//< Manages all the objects in the scene
		InputService*		mInputService = nullptr;					//< Input service for processing input
		IMGuiService*		mGuiService = nullptr;						//< Gui service

		// Entities
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;				//< Pointer to the entity that holds the camera
		ObjectPtr<RenderWindow>	mRenderWindow = nullptr;				//< Pointer to the render window

		// Gui
		std::unique_ptr<FlexblockGui>	mGui = nullptr;
	};
}
