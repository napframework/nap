#pragma once

// External Includes
#include <app.h>
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <imguiservice.h>
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
#include <scene.h>

namespace nap
{
	class RandomApp : public App
	{
		RTTI_ENABLE(App)
	public:
		RandomApp(Core& core) : App(core)	{ }

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
		nap::RenderService*									mRenderService = nullptr;
		nap::ResourceManager*								mResourceManager = nullptr;
		nap::SceneService*									mSceneService = nullptr;
		nap::InputService*									mInputService = nullptr;
		nap::IMGuiService*									mGuiService = nullptr;

		// Resources
		rtti::ObjectPtr<RenderWindow>						mRenderWindow = nullptr;
		rtti::ObjectPtr<Scene>								mScene = nullptr;

		/**
		 * Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);
	};                                                                               
}