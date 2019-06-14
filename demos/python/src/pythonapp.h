#pragma once

// Mod nap render includes
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <app.h>
#include <entityptr.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 * Logs incoming midi and osc messages and allows the user to send a simple OSC value message.
	 * To change the computer the osc message is send to change the ip address in the json (defaults to localhost)
	 */
	class PythonApp : public App
	{
		RTTI_ENABLE(App)
	public:
		PythonApp(nap::Core& core) : App(core)	{ }

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
		RenderService* mRenderService = nullptr;						///< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					///< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							///< Manages all the objects in the scene
		InputService* mInputService = nullptr;							///< Input service for processing input
		IMGuiService* mGuiService = nullptr;							///< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							///< Pointer to the render window
        ObjectPtr<EntityInstance> mMainEntity;
	};
}
