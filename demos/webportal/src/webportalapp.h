/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resourcemanager.h>
#include <renderwindow.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <parameterservice.h>
#include <parametergui.h>
#include <parameterbutton.h>
#include <scene.h>
#include <app.h>
#include <websocketserverendpoint.h>

namespace nap
{
	using namespace rtti;

	/**
	 * The NAP Portal allows you to dynamically create a web-interface to control your NAP application in real-time,
	 * without writing a single line of code.
	 * 
	 * This demo demonstrates how to communicate with the NAP Dashboard.
	 * The NAP-Dashboard uses the NAP Portal NodeJS module to generate a ready-made control interface for your NAP application.
	 *
	 * Instructions on how to install and build the NAP Dashboard can be found here:
	 * https://github.com/napframework/nap-dashboard
	 * Build and run the dashboard using: 'npm run dev'
	 *
	 * Launch this demo and browse to the NAP-Dashboard in your browser. Make sure both environments use the same port for communication.
	 * This demo defaults to port 2000. The port can be modified by editing the 'Port' property of the 'WebSocketServerEndPoint' in portal.json.
	 * If everything works you should see that you are connected, together with a list of parameters that match the group of parameters in this demo.
	 * Changing a parameter in the NAP application will update the control in the browser and vice-versa.
	 *
	 * This demo uses the 'nap::PortalWebSocketServer' to receive and respond to messages received over a web-socket.
	 * Messages are forwarded as 'nap::PortalEvent' to the 'nap::PortalComponent', which in turn forwards the event to the correct 'nap::PortalItem'.
	 * Every item updates a nap::Parameter, which can be rendered as a GUI element and are used to control your application.
	 *
	 * You can add and remove portal items from your NAP application. Changes to the data structure are
	 * reflected in the the web-interface after a refresh. To get a better feeling of how everything works together,
	 * experiment with adding and removing portal items to the 'PortalEntity'.
	 */
	class WebPortalApp : public App
	{
		RTTI_ENABLE(App)
	public:
		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		WebPortalApp(nap::Core& core) : App(core)	{ }

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

		ResourceManager*			mResourceManager = nullptr;		///< Manages all the loaded resources
		RenderService*				mRenderService = nullptr;		///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;		///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;		///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;			///< Manages gui related update / draw calls
		ParameterService*			mParameterService = nullptr;	///< Manages all parameters in the application
		ResourcePtr<ParameterGUI>	mParameterGUI = nullptr;		///< Draws the parameters to screen
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;		///< Pointer to the render window
		ObjectPtr<Scene>			mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<ParameterButton>	mButton = nullptr;				///< Pointer to GUI button
		ObjectPtr<WebSocketServerEndPoint > mServerEndPoint = nullptr;	//< Pointer to the web-socket server endpoint, manages all client
	};
}	
