/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "oscmidi.h"

// External includes
#include <nap/core.h>
#include <nap/logger.h>
#include <mathutils.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>

#include <midihandler.h>
#include <oschandler.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OscMidiApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool OscMidiApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("oscmidi.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

        // Find the main entity
        mMainEntity = scene->findEntity("main");
        
        // Find the OSC sender
        mOscSender = mResourceManager->findObject<OSCSender>("OSCSender");

		// Reserve some memory for the osc output tag
		mOscOutputTag.reserve(512);

		return true;
	}
	
	
	/**
     * Logs all incoming midi messages and all OSC messages coming in through port 7000.
     * Also allows the user to send OSC messages to localhost port 7000 so they can be seen in the log.
	 */
	void OscMidiApp::update(double deltaTime)
	{
		// Find the midi and osc handle components
        auto midi_handler = mMainEntity->findComponent<MidiHandlerComponentInstance>(); 
        
        // Log some information to the top of the display
		ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("Midi and OSC demo");
        ImGui::Text(getCurrentDateTime().toString().c_str());
        ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		// Display midi input messages
        if (ImGui::CollapsingHeader("Midi input log"))
        {
			showMidiLog();
        }

		// Display OSC Input Messages
		if (ImGui::CollapsingHeader("OSC input log"))
		{
			showOSCLog();
		}
        
        // Allow the user to send an OSC value message to a specified address.
        if (ImGui::CollapsingHeader("OSC output"))
        {
			showSendOSC();
        }
        ImGui::End();

	}

	
	/**
	 * Draw the gui + osc / midi messages to screen
	 */
	void OscMidiApp::render()
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

			// Draw our GUI to target
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// Stop recording operations for this window
			mRenderService->endRecording();
		}

		// End frame capture
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void OscMidiApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void OscMidiApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int OscMidiApp::shutdown()
	{
		return 0;
	}


	void OscMidiApp::showMidiLog()
	{
		// Fetch the midi handle component
		auto midi_handler = mMainEntity->findComponent<MidiHandlerComponentInstance>();

		// Get all received osc messages and convert into a single string
		std::string msg;
		for (const auto& message : midi_handler->getMessages())
			msg += (message + "\n");

		// Backup text
		char txt[256] = "No Midi Messages Received";

		// If there are no messages display that instead of the received messages
		char* display_msg = msg.empty() ? txt : &msg[0];
		size_t display_size = msg.empty() ? 256 : msg.size();

		// Display block of text
		ImGui::InputTextMultiline("Midi Messages", display_msg, display_size, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 15), ImGuiInputTextFlags_ReadOnly);
	}


	void OscMidiApp::showOSCLog()
	{
		// Get the osc handle component
		auto osc_handler = mMainEntity->findComponent<OscHandlerComponentInstance>();

		// Get all received osc messages and convert into a single string
		std::string msg;
		for (const auto& message : osc_handler->getMessages())
			msg += (message + "\n");

		// Backup text
		char txt[256] = "No OSC Messages Received";

		// If there are no messages display that instead of the received messages
		char* display_msg = msg.empty() ? txt : &msg[0];
		size_t display_size = msg.empty() ? 256 : msg.size();

		// Display block of text
		ImGui::InputTextMultiline("OSC Messages", display_msg, display_size, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 15), ImGuiInputTextFlags_ReadOnly);
	}


	void OscMidiApp::showSendOSC()
	{
		std::string display_string = utility::stringFormat("Send OSC message to: %s, port: %d, with the specified address and value", mOscSender->mIPAddress.c_str(), mOscSender->mPort);
		ImGui::Text(display_string.c_str());
		ImGui::InputText("Address", &mOscOutputTag[0], mOscOutputTag.capacity());
		ImGui::SliderFloat("Value", &mOscOutputValue, 0.f, 1.f, "%.3f", 1);
		if (ImGui::Button("Send"))
		{
			std::string address = "/" + std::string(mOscOutputTag.data());
			OSCEvent event(address);
			event.addValue<float>(mOscOutputValue);
			mOscSender->send(event);
		}
	}

}
