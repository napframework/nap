#include "oscmidi.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <mathutils.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>

#include <midieventqueue.h>

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
        
        // Initialized the list of last arrived midi events
        for (auto i = 0; i < 10; ++i)
            mMidiEventList.emplace_back(nullptr);
        
		return true;
	}
	
	
	/**
	 */
	void OscMidiApp::update(double deltaTime)
	{
        auto midiQueueComponent = mMainEntity->findComponent<MidiEventQueueComponentInstance>();
        
        // Poll the midi event queue for incoming events
        std::vector<std::unique_ptr<MidiEvent>> queue;
        if (midiQueueComponent)
            queue = midiQueueComponent->poll();
        
        // Update the list of recent events to be displayed
        for (auto& event : queue)
        {
            for (auto i = mMidiEventList.size() - 1; i > 0; --i)
                mMidiEventList[i] = std::move(mMidiEventList[i - 1]);
            mMidiEventList[0] = std::move(event);
        }
        
		// Draw some gui elements
		ImGui::Begin("Midi");
        
        // Draw the list of last arrived events in reversed order
        for (int i = mMidiEventList.size() - 1; i >= 0; i--)
            if (mMidiEventList[i] != nullptr)
                ImGui::Text(mMidiEventList[i]->toString().c_str());
            
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
