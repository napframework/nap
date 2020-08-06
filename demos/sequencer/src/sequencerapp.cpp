// Local Includes
#include "sequencerapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <napcolors.h>
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
		
		// Fetch the scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Convert our path and load resources from file
        auto abspath = utility::getAbsolutePath(mFilename);
        nap::Logger::info("Loading: %s", abspath.c_str());
        if (!mResourceManager->loadFile(mFilename, error))
            return false;

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		mSequenceEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
		if (!error.check(mSequenceEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
			return false;

		mParameterGroup = mResourceManager->findObject<ParameterGroup>("ParameterGroup");
		if (!error.check(mParameterGroup != nullptr, "unable to find ParameterGroup with name: %s", "ParameterGroup"))
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

		// Create the parameter GUI, automatically shows a group of parameters in a window
		mParameterGUI = std::make_unique<ParameterGUI>(*mParameterService);

		// All done!
        return true;
    }

	
	/**
	 * Forwards the received mouse and keyboard input events and shows the sequencer gui elements,
	 * together with some general application information and the parameters.
	 */
	void SequencerApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Show parameters
		mParameterGUI->show(mParameterGroup.get(), true);

		// Show general application information
		ImGui::Begin("Information");
		ImGui::SameLine();
		getCore().getFramerate();
		ImGui::TextColored(mTextHighlightColor.convert<RGBAColorFloat>(), "%.3f ms/frame (%.1f FPS)", 1000.0f / getCore().getFramerate(), getCore().getFramerate());
		ImGui::End();

		// Show sequence editor GUI
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

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Draw GUI to screen
			mGuiService->draw();

			// End the render pass
			mRenderWindow->endRendering();

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
				mRenderWindow->toggleFullscreen();
		}

		// Forward to input service
		mInputService->addEvent(std::move(inputEvent));
    }


    int SequencerApp::shutdown()
    {
		return 0;
    }
}