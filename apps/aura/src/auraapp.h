#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <etherdreamservice.h>
#include <oscservice.h>
#include <imguiservice.h>
#include <app.h>
#include <scene.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class AuraApp : public App
	{
		RTTI_ENABLE(App)
	public:
		AuraApp(nap::Core& core) : App(core)		{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& error) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Update is called before render
		 */
		void update(double deltaTime) override;

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
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		EtherDreamService* mLaserService = nullptr;						//< Laser service
		OSCService* mOscService = nullptr;								//< Laser DAC
		IMGuiService* mGUIService = nullptr;							//< Gui Service

		rtti::ObjectPtr<RenderWindow> mRenderWindow = nullptr;			//< Pointers to the render window// Laser DAC
		rtti::ObjectPtr<EntityInstance> mLaserController = nullptr;		//< Entity that holds all the lasers to update / draw
		rtti::ObjectPtr<EntityInstance> mLaserCamera = nullptr;			//< Entity that holds the camera that is used to render the laser to a backbuffer
		rtti::ObjectPtr<EntityInstance> mFrameCamera = nullptr;			//< Entity that holds the camera that is used to render all the backbuffers to screen
		rtti::ObjectPtr<Scene> mScene = nullptr;						//< Nap scene, contains all entities
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
	};
}
