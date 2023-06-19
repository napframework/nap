/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpsendapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <inputrouter.h>
#include <imguiutils.h>
#include <apimessage.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPSendApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
    // Creates an API message with and APIString as argument containing messageContent as value, returns true upon
    // successful creation, returns false upon failure and errorState contains error description
    static bool createAPIMessage(std::string& serializedAPIMessage, const std::string& messageContent, utility::ErrorState& errorState);

	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool UDPSendApp::init(utility::ErrorState& error)
	{		
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
        mUDPClient = mResourceManager->findObject("UDPClient");

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The camera has two input components: KeyInputComponent and PointerInputComponent.
	 * The key input component receives key events, the pointer input component receives pointer events.
	 */
	void UDPSendApp::update(double deltaTime)
	{
		// Create an input router, the default one forwards messages to mouse and keyboard input components
		nap::DefaultInputRouter input_router;
		
		// Now forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Setup GUI
		ImGui::Begin("UDP Send");
        ImGui::Text(getCurrentDateTime().toString().c_str());
        ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "Run UDP Receive demo to receive the message.");
        ImGui::TextColored(mGuiService->getPalette().mHighlightColor3, utility::stringFormat("Client Port: %d", mUDPClient->mPort).c_str());
        ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

        ImGui::Spacing();

        // Input box to change message content
        ImGui::InputTextMultiline("", &mMessageContent, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10));

        // Send message content as APIMessage
        if(ImGui::Button("Send"))
        {
            // Create API message and serialize it as a JSON string, log any errors
            utility::ErrorState error_state;
            std::string data;
            if(createAPIMessage(data, mMessageContent, error_state))
            {
                mUDPClient->send(data);
            }else
            {
                nap::Logger::error(error_state.toString());
            }
        }

		ImGui::End();
	}

	
	/**
	 * Renders the GUI
	 */
	void UDPSendApp::render()
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
	void UDPSendApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void UDPSendApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int UDPSendApp::shutdown()
	{
		return 0;
	}


    bool createAPIMessage(std::string& serializedAPIMessage, const std::string& messageContent, utility::ErrorState& errorState)
    {
        // Create API message with unique UUID
        APIMessage api_message;
        api_message.mName = "APIMessage";
        api_message.mID = "Message";

        // Fill the message with a single API string containing message content
        auto api_value = APIString("Content", messageContent);
        api_value.mID = math::generateUUID();
        api_message.mArguments.emplace_back(&api_value);

        // Serialize message to JSON
        if(api_message.toJSON(serializedAPIMessage, errorState))
        {
            return true;
        }

        return false;
    }
}
