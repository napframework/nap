/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "sequencerapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <sequenceplayereventoutput.h>
#include <sequenceevent.h>

namespace nap 
{    
    bool SequencerApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService		= getCore().getService<nap::RenderService>();
		mSceneService		= getCore().getService<nap::SceneService>();
		mInputService		= getCore().getService<nap::InputService>();
		mGuiService			= getCore().getService<nap::IMGuiService>();
		mParameterService	= getCore().getService<nap::ParameterService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();

		// Fetch parameter GUI
		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");
		if (!error.check(mParameterGUI != nullptr, "unable to find parameter GUI"))
			return false;

		// Get the time-line window
		mTimelineWindow = mResourceManager->findObject<nap::RenderWindow>("SequencerWindow");
		if (!error.check(mTimelineWindow != nullptr, "unable to find SequencerWindow"))
			return false;

		// Get the parameter window
		mParameterWindow = mResourceManager->findObject<nap::RenderWindow>("ParameterWindow");
		if (!error.check(mParameterWindow != nullptr, "unable to find ParameterWindow"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the sequence editor gui
		mSequenceEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
		if (!error.check(mSequenceEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
			return false;

		const ObjectPtr<SequencePlayerEventOutput> eventOutput = mResourceManager->findObject<SequencePlayerEventOutput>("SequencePlayerEventOutput");
		if (!error.check(eventOutput != nullptr, "unable to find SequenceEventReceiver with name: %s", "SequencePlayerEventOutput"))
			return false;

		eventOutput->mSignal.connect([](const SequenceEventBase& event)
	    {
			 static std::unordered_map<rttr::type, void(*)(const SequenceEventBase&)> sHandleEventMap
			{
				{
				 	RTTI_OF(SequenceEventString),
					[](const SequenceEventBase& event)
					{
					  const SequenceEventString& eventString = static_cast<const SequenceEventString&>(event);
					  nap::Logger::info("Event received with value : %s", eventString.getValue().c_str());
				 	}
		 		},
				{
					RTTI_OF(SequenceEventFloat),
					[](const SequenceEventBase& event)
					{
					  const SequenceEventFloat& eventFloat = static_cast<const SequenceEventFloat&>(event);
					  nap::Logger::info("Event received with value : %f", eventFloat.getValue());
					}
				},
				{
					RTTI_OF(SequenceEventInt),
					[](const SequenceEventBase& event)
					{
					  const SequenceEventInt& eventInt = static_cast<const SequenceEventInt&>(event);
					  nap::Logger::info("Event received with value : %i", eventInt.getValue());
					}
				},
				{
					RTTI_OF(SequenceEventVec2),
					[](const SequenceEventBase& event)
					{
					  const SequenceEventVec2& eventVec2 = static_cast<const SequenceEventVec2&>(event);
					  nap::Logger::info("Event received with value : %f %f", eventVec2.getValue().x,
										eventVec2.getValue().y);
					}
				},
				{
					RTTI_OF(SequenceEventVec3),
					[](const SequenceEventBase& event)
					{
					  const SequenceEventVec3& eventVec3 = static_cast<const SequenceEventVec3&>(event);
					  nap::Logger::info("Event received with value : %f %f %f", eventVec3.getValue().x,
										eventVec3.getValue().y, eventVec3.getValue().z);
					}
				}
			};

			 auto it = sHandleEventMap.find(event.get_type());
			 assert(it!=sHandleEventMap.end()); // type not found
			 if(it!=sHandleEventMap.end())
			 {
				 it->second(event);
			 }
		});

		// All done!
        return true;
    }

	
	/**
	 * Forwards the received mouse and keyboard input events and shows the sequencer gui elements,
	 * together with some general application information and the parameters.
	 */
	void SequencerApp::update(double deltaTime)
	{
		// Forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mParameterWindow, input_router,	{ &mScene->getRootEntity()});

		// Show parameters
		mGuiService->selectWindow(mParameterWindow);
		
		// Show all parameters
		ImGui::Begin("Parameters");
		mParameterGUI->show(false);

		// Display some extra info
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"Play the sequence to animate the parameters");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();

		// Show sequence editor GUI
		mGuiService->selectWindow(mTimelineWindow);
		mSequenceEditorGUI->show();
	}


    // Called when the application is going to render.
	// Draws the gui to the main window.
	void SequencerApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Draw GUI parameter window
		if (mRenderService->beginRecording(*mParameterWindow))
		{
			// Begin the render pass
			mParameterWindow->beginRendering();

			// Draw GUI to screen
			mGuiService->draw();

			// End the render pass
			mParameterWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Draw GUI timeline window
		if (mRenderService->beginRecording(*mTimelineWindow))
		{
			// Begin the render pass
			mTimelineWindow->beginRendering();

			// Draw GUI to screen
			mGuiService->draw();

			// End the render pass
			mTimelineWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
    }


    void SequencerApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void SequencerApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// Exit when esc is pressed
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mTimelineWindow->toggleFullscreen();
		}

		// Forward to input service
		mInputService->addEvent(std::move(inputEvent));
    }


    int SequencerApp::shutdown()
    {
		return 0;
    }
}
