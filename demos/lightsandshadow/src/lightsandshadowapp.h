/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderadvancedservice.h>
#include <renderablemeshcomponent.h>
#include <renderbloomcomponent.h>
#include <rendertotexturecomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <rendertarget.h>
#include <app.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward declares
	class LightComponentInstance;
	class PerspCameraComponentInstance;

	/**
	 * Demo application that renders a scene of geometric objects with three lights of distinct types that also cast shadows.
	 * 
	 * The layer setup is such that the bounds are always rendered first, then scene objects, and finally a fader component
	 * that acts as a color overlay that can fade in to and out from black. Objects that are included in the shadow map are
	 * distinguished using the `Shadow` tag. The RenderAdvanced service call `renderShadows` is responsible for most of the
	 * heavy lifting as it updates the shadow maps of all lights and updates the uniform and sampler information of all
	 * objects. When rendering our scene to the window using `RenderService::renderObjects`, all of the information is in
	 * place for the system to resolve and execute the render passes appropriately.
	 */
	class LightsAndShadowApp : public App
	{
		RTTI_ENABLE(App)
	public:
		LightsAndShadowApp(nap::Core& core) : App(core) { }

		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all render-able objects to the GPU
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

	private:
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		RenderAdvancedService* mRenderAdvancedService = nullptr;		//< Render Advanced Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< IMGui service

		rtti::ObjectPtr<RenderWindow> mRenderWindow;					//< Pointers to the render window
		rtti::ObjectPtr<EntityInstance> mDefaultInputRouter;			//< Routes input events to the input component
		rtti::ObjectPtr<EntityInstance> mCameraEntity;					//< Entity that holds the camera
		rtti::ObjectPtr<EntityInstance> mWorldEntity;					//< World entity
		rtti::ObjectPtr<EntityInstance> mSpotLightEntity;				//< Spotlight entity
		rtti::ObjectPtr<EntityInstance> mSunLightEntity;				//< Sunlight entity
		rtti::ObjectPtr<EntityInstance> mPointLightEntity;				//< Pointlight entity

		rtti::ObjectPtr<RenderTag> mShadowTag = 0;						//< Shadow tag
		bool mShowLocators = false;										///< If light origin is shown
		bool mShowFrustrum = true;										///< If light frustrum is shown
	};
}
