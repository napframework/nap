/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <app.h>
#include <udpclient.h>
#include <parametercolor.h>
#include <parameter.h>
#include <parametersimple.h>
#include <parametergui.h>

namespace nap
{
	using namespace rtti;

    /**
     * Demo application that demonstrates the use of UDPClient that sends raw UDP packets.
	 * 
     * The application converts the values of a Color Parameter and a String Parameter to nap API messages,
	 * using the UDPSendComponent. API messages are serialized and send as UDP packets using the UDPClient resource.
	 * 
     * Run the UDP receive demo to observe the changes.
     * Use Napkin to change properties like the IP & Port of the UDPClient
     */
	class UDPSendApp : public App
	{
		RTTI_ENABLE(App)
	public:
		UDPSendApp(nap::Core& core) : App(core)	{ }

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
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window
        ObjectPtr<ParameterRGBColor8> mParameterColor;                  //< Pointer to Parameter Color
        ObjectPtr<ParameterString> mParameterString;                    //< Pointer to Parameter String
        ObjectPtr<UDPClient> mUDPClient;                                //< Pointer to UDP client
        ObjectPtr<ParameterGUI> mParameterGUI;                          //< Pointer to Parameter GUI
	};
}
