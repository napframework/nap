#include "oscmidi.h"

// Nap includes
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
        
        // Initialized the list of last arrived midi events
        for (auto i = 0; i < 4; ++i)
        {
            mMidiMessageList.emplace_back("");
            mOscMessageList.emplace_back("");
        }
        
        std::string temp  = "address";
        memcpy(mOscOutputTag.data(), temp.c_str(), temp.size());
        

		return true;
	}
	
	
	/**
     * Logs all incoming midi messages and all OSC messages coming in through port 7000.
     * Also allows the user to send OSC messages to localhost port 7000 so they can be seen in the log.
	 */
	void OscMidiApp::update(double deltaTime)
	{
        auto midiHandler = mMainEntity->findComponent<MidiHandlerComponentInstance>();
        
        // Poll the midi event queue for incoming events
        std::vector<std::string> midiMessages;
        if (midiHandler)
            midiMessages = midiHandler->poll();
        
        // Update the list of recent events to be displayed
        for (auto& message : midiMessages)
        {
            mMidiMessageList[mMidiMessageListWriteIndex] = message;
            mMidiMessageListWriteIndex++;
            if (mMidiMessageListWriteIndex >= mMidiMessageList.size())
                mMidiMessageListWriteIndex = 0;
        }
        
        auto oscHandler = mMainEntity->findComponent<OscHandlerComponentInstance>();

        // Poll the OSC event queue for incoming events
        std::vector<std::string> oscMessages;
        if (oscHandler)
            oscMessages = oscHandler->poll();

        // Update the list of recent OSC events to be displayed
        for (auto& message : oscMessages)
        {
            mOscMessageList[mOscMessageListWriteIndex] = message;
            mOscMessageListWriteIndex++;
            if (mOscMessageListWriteIndex >= mOscMessageList.size())
                mOscMessageListWriteIndex = 0;
        }
        
        // Log all incoming midi messages
        ImGui::Begin("Midi and OSC demo");
        ImGui::Text(utility::getCurrentDateTime().toString().c_str());
        ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

        if (ImGui::CollapsingHeader("Midi input log"))
        {
            for (int i = 0; i < mMidiMessageList.size(); i++)
            {
                auto index = (mMidiMessageListWriteIndex + i) % mMidiMessageList.size();
                if (mMidiMessageList[index] != "")
                    ImGui::Text(mMidiMessageList[index].c_str());
            }
            for (int i = 0; i < mMidiMessageList.size(); i++)
                if (mMidiMessageList[i] == "")
                    ImGui::Text("");
        }
        
        // Log OSC messages coming in through port 7000
        if (ImGui::CollapsingHeader("OSC input log"))
        {
            for (int i = 0; i < mOscMessageList.size(); i++)
            {
                auto index = (mOscMessageListWriteIndex + i) % mOscMessageList.size();
                if (mOscMessageList[index] != "")
                    ImGui::Text(mOscMessageList[index].c_str());
            }
            for (int i = 0; i < mOscMessageList.size(); i++)
                if (mOscMessageList[i] == "")
                    ImGui::Text("");
        }
        
        // Allow the user to send an OSC value message to a specified address.
        if (ImGui::CollapsingHeader("OSC output"))
        {
            ImGui::Text("Send an OSC message to localhost port 7000 with the specified address and value");
            ImGui::InputText("Address", mOscOutputTag.data(), 25);
            ImGui::SliderFloat("Value", &mOscOutputValue, 0.f, 1.f, "%.3f", 1);
            if (ImGui::Button("Send"))
            {
                std::string address = "/" + std::string(mOscOutputTag.data());
                OSCEvent event(address);
                event.addValue<float>(mOscOutputValue);
                mOscSender->send(event);
            }
        }
        
        ImGui::End();

	}

	
	/**
	 */
	void OscMidiApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Draw our gui
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
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
}
