#pragma once

// Local Includes
#include "firstpersoncontroller.h"
#include "pointlightcomponent.h"
#include "orbitcontroller.h"
#include "cameracomponent.h"
#include "cameracontroller.h"
#include "artnetmeshfromfile.h"

// External Includes
#include <app.h>
#include <sceneservice.h>
#include <videoservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/logger.h>
#include <rendertarget.h>
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <nap/windowresource.h>
#include <inputrouter.h>
#include <entity.h>
#include <video.h>
#include <artnetcontroller.h>

namespace nap
{
	class NAPAPI KalvertorenApp : public App
	{
		RTTI_ENABLE(App)
	public:
		KalvertorenApp(Core& core) : App(core)	{ }

		/**
		 *	Initialize the kalvertoren app and it's resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update the kalvertoren app's resources
		 */
		virtual void update(double deltaTime);

		/**
		 *	Render stuff to screen
		 */
		virtual void render();

		/**
		 *	Shut down the application
		 */
		virtual void shutdown();

		/**
		 *	Register input events
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;

		/**
		 *	Register window events
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;

		/**
		 *	Applies the rendered video texture to the vertices as color
		 */
		void applyVideoTexture(ArtnetMeshFromFile& mesh);

		/**
		 *	Updates the gui
		 */
		void updateGui();

	private:
		// Nap Objects
		nap::RenderService*								renderService = nullptr;
		nap::ResourceManager*							resourceManager = nullptr;
		nap::SceneService*								sceneService = nullptr;
		nap::VideoService*								videoService = nullptr;
		nap::InputService*								inputService = nullptr;

		nap::ObjectPtr<nap::RenderWindow>				renderWindow;
		nap::ObjectPtr<nap::RenderTarget>				videoTextureTarget;
		nap::ObjectPtr<nap::EntityInstance>				ledEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				sceneCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				videoCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				defaultInputRouter = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				videoEntity = nullptr;
		nap::ObjectPtr<nap::Video>						videoResource = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				lightEntity = nullptr;
		nap::ObjectPtr<nap::Material>					frameMaterial = nullptr;
		nap::ObjectPtr<nap::Material>					vertexMaterial = nullptr;
		nap::ObjectPtr<nap::EntityInstance>				displayEntity = nullptr;

		// video data
		opengl::Bitmap									mVideoBitmap;

		// GUI
		int												mMeshSelection = 0;
		int												mPaintMode = 0;
		int												mSelectChannel = 0;
		float											mChannelSpeed = 1.0f;
		bool											mFirst = true;
	};
}