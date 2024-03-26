/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "webportalapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <inputrouter.h>

// Register this application with RTTI, this is required by the AppRunner to
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebPortalApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool WebPortalApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService		= getCore().getService<nap::RenderService>();
		mSceneService		= getCore().getService<nap::SceneService>();
		mInputService		= getCore().getService<nap::InputService>();
		mGuiService			= getCore().getService<nap::IMGuiService>();
		mParameterService	= getCore().getService<nap::ParameterService>();

		// Get the resource manager
		mResourceManager = getCore().getResourceManager();

		// Get the parameter GUI
		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");
		if (!error.check(mParameterGUI != nullptr, "unable to find parameter GUI with name: %s", "ParameterGUI"))
			return false;
		   
		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Server end point, handles connections
		mServerEndPoint = mResourceManager->findObject<nap::UnsecureWebSocketServerEndPoint>("WebSocketServerEndPoint");
		if (!error.check(mServerEndPoint != nullptr, "unable to find server end point with name WebSocketServerEndPoint"))
			return false;

		// Get button from the GUI and connect to its signals
		mButton = mResourceManager->findObject<nap::ParameterButton>("ParameterButton");
		if (!error.check(mButton != nullptr, "unable to find button with name: %s", "Button"))
			return false;

		// Connect button callbacks
		mButton->click.connect([]() {
			nap::Logger::info("Button was clicked");
			});

		mButton->press.connect([]() {
			nap::Logger::info("Button was pressed");
			});


		mButton->release.connect([]() {
			nap::Logger::info("Button was released");
			});

		// All done!
		return true;
	}


	/**
	 * Called when the window is updating
	 */
	void WebPortalApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Setup the connectivity GUI
		ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(640, 0), ImGuiCond_Once);

		ImGui::Begin("Web Portal Demo");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "Connect a web-portal to control the parameters in this window.");
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor3, utility::stringFormat("Server Port: %d", mServerEndPoint->mPort).c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			mParameterGUI->show(false);
		}
		if (ImGui::CollapsingHeader("Connected Clients"))
		{
			// Get all connected clients
			std::vector<std::string> host_names;
			mServerEndPoint->getHostNames(host_names);

			// Combine for display
			std::string msg;
			for (const auto& name : host_names)
				msg += (name + "\n");

			if (msg.empty())
				msg = "No Connected Clients";

			// Display block of text
			ImGui::InputTextMultiline("Clients", &msg[0], msg.size(), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 15), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::End();
	}


	/**
	 * Called when the window is going to render
	 */
	void WebPortalApp::render()
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

			// Render GUI elements
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
	 * Called when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window.
	 * On the next update the render service automatically processes all window events.
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal.
	 */
	void WebPortalApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on.
	 * In this case we also check if we need to toggle full-screen or exit the running app.
	 */
	void WebPortalApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());

			// When 'esc' is pressed, quit the app
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// When 'f' is pressed, toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int WebPortalApp::shutdown()
	{
		return 0;
	}
}
