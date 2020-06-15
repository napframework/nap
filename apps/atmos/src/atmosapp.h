#pragma once

// Local Includes
#include "atmosgui.h"
#include "applysensorcomponent.h"

// Nap includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>
#include <smoothdamp.h>
#include <rendertarget.h>
#include <yoctosensor.h>

//ruud noobing:
#include <switchpresetcomponent.h>

namespace nap
{
	using namespace rtti;

	/**
	 *	Atmos visualization application
	 */
	class AtmosApp : public App
	{
		RTTI_ENABLE(App)
		friend class AtmosGui;
	public:
		AtmosApp(nap::Core& core) : App(core)	{ }

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

		// Objects
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			//< Pointer to the render window
		ObjectPtr<RenderTarget>		mVideoTarget  = nullptr;			//< Pointer to the video render target

		// Entities
		ObjectPtr<EntityInstance>		mUniverseEntity = nullptr;		///< Pointer to the universe entity

		ObjectPtr<EntityInstance>		mCameraEntity		= nullptr;		///< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance>		mWorldEntity		= nullptr;		///< Pointer to the world entity
		ObjectPtr<EntityInstance>		mScanEntity			= nullptr;		///< Pointer to the scan entity (living under the world)
		ObjectPtr<EntityInstance>		mLineEntity			= nullptr;		///< Pointer to the cam path entity
		ObjectPtr<EntityInstance>		mVideoEntity		= nullptr;		///< Pointer to the video entity
		ObjectPtr<EntityInstance>		mVideoCameraEntity	= nullptr;		///< Pointer to the camera that renders the video
		ObjectPtr<EntityInstance>		mSensorEntity		= nullptr;		///< Pointer to the sensor entity that handles sensory input		
		
		ObjectPtr<YoctoProximitySensor> mProximitySensor	= nullptr;		///< Yoctopuce proximity sensor

		//
		ApplySensorComponent*			mApplySensorComponent = nullptr;

		// Gui
		std::unique_ptr<AtmosGui>	mGui = nullptr;
	};
}
