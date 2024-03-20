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
#include <parametergui.h>
#include <parameternumeric.h>
#include <nap/signalslot.h>
#include <cubemapfromfile.h>

namespace nap
{
	// Forward declares
	class LightComponentInstance;
	class PerspCameraComponentInstance;

	/**
	 * Demo application that renders a sky box and a reflective 3D object.
	 *
	 * This app uses `naprenderadvanced` object `nap::RenderSkyBoxComponent` with a `nap::SkyBoxShader` to render a skybox.
	 * The `nap::TorusMesh` is rendered with a simple custom shader that samples the same cube map that is used for the
	 * skybox to display a (fake) reflection. Additionally, we create a simple combobox GUI in the `SkyBoxApp::update` hook
	 * that allows the user to switch between cube maps. Here we make sure to update the sampler instances of both the skybox
	 * and the torus.
	 *
	 * We also demonstrate how to use render layers to your advantage `nap::RenderLayer`. A `nap::RenderLayerRegistry` defines
	 * two layers `SkyBox` and `Default` where the skybox layer is the bottom layer, and therefore rendered first. 
	 * The skybox does not read or write any depth information, making it more efficient to render.
	 */
	class SkyBoxApp : public App
	{
		RTTI_ENABLE(App)
	public:
		SkyBoxApp(nap::Core& core) : App(core) { }

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
		rtti::ObjectPtr<EntityInstance> mTorusEntity;					//< The torus entity
		rtti::ObjectPtr<EntityInstance> mSkyboxEntity;					//< Sky box render entity

		std::vector<rtti::ObjectPtr<CubeMapFromFile>> mCubeMaps;		//< Cube maps cache
		int mCubeMapIndex = 0;
	};
}
