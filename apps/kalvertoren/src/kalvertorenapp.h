#pragma once

// Local Includes
#include "firstpersoncontroller.h"
#include "pointlightcomponent.h"
#include "orbitcontroller.h"
#include "cameracomponent.h"
#include "cameracontroller.h"
#include "artnetmeshfromfile.h"
#include "kalvertorengui.h"

// External Includes
#include <app.h>
#include <sceneservice.h>
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
#include <artnetcontroller.h>
#include <utility/datetimeutils.h>

namespace nap
{
	class KalvertorenGui;

	class KalvertorenApp : public App
	{
		RTTI_ENABLE(App)
		friend class KalvertorenGui;
	public:
		KalvertorenApp(Core& core) : App(core)	{ }

		/**
		 *	Initialize the kalvertoren app and it's resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update the kalvertoren app's resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Render stuff to screen
		 */
		virtual void render() override;

		/**
		 *	Shut down the application
		 */
		virtual int shutdown() override;

		/**
		 *	Register input events
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;

		/**
		 *	Register window events
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;

	private:
		// Nap Objects
		nap::RenderService*									renderService = nullptr;
		nap::ResourceManager*								resourceManager = nullptr;
		nap::SceneService*									sceneService = nullptr;
		nap::InputService*									inputService = nullptr;

		nap::ObjectPtr<nap::RenderWindow>					renderWindow;
		nap::ObjectPtr<nap::EntityInstance>					compositionEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					sceneCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					compositionCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					renderCompositionEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					defaultInputRouter = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					lightEntity = nullptr;
		nap::ObjectPtr<nap::Material>						frameMaterial = nullptr;
		nap::ObjectPtr<nap::Material>						vertexMaterial = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					displayEntity = nullptr;

		std::unique_ptr<KalvertorenGui>						mGui;

		/**
		 * Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);
	};                                                                               
}