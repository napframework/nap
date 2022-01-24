/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pythonapp.h"

// External includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <pythonscriptcomponent.h>
#include <renderservice.h>

// Register this application with RTTI, this is required by the AppRunner to
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PythonApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool PythonApp::init(utility::ErrorState& error)
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

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

        // Find the entity containing the PythonScriptComponent
        mPythonEntity = scene->findEntity("PythonEntity");
        if (mPythonEntity == nullptr)
        {
            error.fail("PythonEntity not found");
            return false;
        }

		// Store the current python value
        auto pythonComponent = mPythonEntity->findComponent<PythonScriptComponentInstance>();
        if (!pythonComponent->get<float>("getValue", error, mValue)) // Initialize mValue using the getter in the python script.
            return false;

		return true;
	}


	/**
	 * Update call of the application. Called every frame. 
	 * Uses a couple of gui elements to send and receive values from a pythons script.
	 */
	void PythonApp::update(double deltaTime)
	{
        // Log some information to the top of the display
		ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("Python demo");

		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(mGuiService->getColors().mHighlightColor2, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		if (ImGui::CollapsingHeader("Controls"))
		{
			// Get the python script component
			auto pythonComponent = mPythonEntity->findComponent<PythonScriptComponentInstance>();

			// When printing is turned on python prints the received message to console
			utility::ErrorState errorState;
			if (ImGui::Checkbox("Python Print", &mPythonPrint))
			{
				if (!pythonComponent->call("printToConsole", errorState, mPythonPrint))
					nap::Logger::warn(errorState.toString());
			}

			// The variable mValue is controlled by a slider.
			// The setter setValue() within the python script is called to set a member variable within python.
			// The getter getValue() is called to retrieve the python member's value and to display it/
			ImGui::Text("Call the PythonScriptComponent's setter using the slider:");
			if (ImGui::SliderFloat("New Value", &mValue, 0.f, 1.f))
			{
				// Call setValue() method within the python script.
				if (!pythonComponent->call("setValue", errorState, mValue))
					nap::Logger::warn(errorState.toString());
			}

			// Get the value from the python script
			float returnValue = 0;
			if (!pythonComponent->get<float>("getValue", errorState, returnValue)) // Call getValue() method within the python script.
				nap::Logger::warn(errorState.toString());

			ImGui::Text("The PythonScript returns: ");
			ImGui::SameLine();
			ImGui::TextColored(mGuiService->getColors().mHighlightColor2, utility::stringFormat("%f", returnValue).c_str());
		}
        ImGui::End();

	}


	/**
	 * Draw the gui + osc / midi messages to screen
	 */
	void PythonApp::render()
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
	void PythonApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void PythonApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int PythonApp::shutdown()
	{
		return 0;
	}


}
