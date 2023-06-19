/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpreceiveapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <uniforminstance.h>
#include <imguiutils.h>
#include <apimessage.h>
#include <mathutils.h>
#include <udpreceivecomponent.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPReceiveApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool UDPReceiveApp::init(utility::ErrorState& error)
	{		
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
        mAPIService     = getCore().getService<nap::APIService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

        // Obtain UDP Server
        mUDPServer = mResourceManager->findObject<UDPServer>("UDPServer");
        if(!error.check(mUDPServer!= nullptr, "UDPServer not found!"))
            return false;

        // Find UDP Entity
        mUDPEntity = scene->findEntity("UDPEntity");
        if(!error.check(mUDPEntity!= nullptr, "UDPEntity not found!"))
            return false;

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The camera has two input components: KeyInputComponent and PointerInputComponent.
	 * The key input component receives key events, the pointer input component receives pointer events.
	 */
	void UDPReceiveApp::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;
		
		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Setup GUI
		ImGui::Begin("UDP Receive");
        ImGui::Text(getCurrentDateTime().toString().c_str());
        ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "Run UDP Send demo to receive the message and display it below.");
        ImGui::TextColored(mGuiService->getPalette().mHighlightColor3, utility::stringFormat("Server Port: %d", mUDPServer->mPort).c_str());
        ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

        ImGui::Spacing();

        // Obtain UDPReceiveComponent from UDP Entity
        auto& udp_receive_comp = mUDPEntity->getComponent<UDPReceiveComponentInstance>();

        // Copy last received message
        std::string last_received_message = udp_receive_comp.getLastReceivedMessage();

        // Display last received message
        ImGui::Text("Last Received Message");
        ImGui::PushID(0);
        ImGui::InputText("", &last_received_message[0], ImGuiInputTextFlags_ReadOnly);
        ImGui::PopID();
        ImGui::Spacing();

        // Copy last received data
        std::string last_received_data = udp_receive_comp.getLastReceivedData();

        // Display last received data
        ImGui::Text("Last Received Data");
        ImGui::PushID(1);
        ImGui::InputTextMultiline("", &last_received_data, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 20),
                                  ImGuiInputTextFlags_ReadOnly);
        ImGui::PopID();
		ImGui::End();
	}

	
	/**
	 * Renders the world and text.
	 */
	void UDPReceiveApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Draw our GUI
			mGuiService->draw();
			
			// End the render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void UDPReceiveApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void UDPReceiveApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int UDPReceiveApp::shutdown()
	{
		return 0;
	}
}
