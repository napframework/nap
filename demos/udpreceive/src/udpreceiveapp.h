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
#include <udpserver.h>
#include <apimessage.h>
#include <apiservice.h>
#include <parametercolor.h>

namespace nap
{
	using namespace rtti;

    /**
     * Demo application that demonstrates the use of UDPServer & APIComponent.
	 * 
     * The demo comes with a UDPReceiveComponent which relays incoming UDP messages to the API service which in turn
     * tries to convert incoming packets to NAP API events changing parameter values.
     * Use Napkin to change properties like the Port & UDP Thread and the API signatures
     */
	class UDPReceiveApp : public App
	{
		RTTI_ENABLE(App)
	public:
        UDPReceiveApp(nap::Core& core) : App(core)	{ }

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
		RenderService* mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;				//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;						//< Manages all the objects in the scene
		InputService* mInputService = nullptr;						//< Input service for processing input
		IMGuiService* mGuiService = nullptr;						//< Manages gui related update / draw calls
        APIService* mAPIService = nullptr;							//< Manages API messages and API components
        ObjectPtr<EntityInstance> mTextEntity = nullptr;			//< Pointer to the entity that can display text
		ObjectPtr<RenderWindow> mRenderWindow;						//< Pointer to the render window
        ObjectPtr<EntityInstance> mUDPEntity = nullptr;				//< Pointer to the entity that holds the UDPReceiveComponent
        ObjectPtr<UDPServer> mUDPServer = nullptr;					//< Pointer to UDPServer
        ObjectPtr<ParameterRGBColor8> mParameterColor = nullptr;	//< Color that is received
        ObjectPtr<ParameterString> mParameterText = nullptr;		//< Message that is received
        std::string mMessage = "";									//< Cached message
	};
}
