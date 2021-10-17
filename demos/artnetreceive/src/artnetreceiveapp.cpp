/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetreceiveapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <artnethandler.h>
#include <algorithm>
#include <sstream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtNetReceiveApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool ArtNetReceiveApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
		mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the Art-Net entity
		mArtNetEntity = mScene->findEntity("ArtNetEntity");
		if (!error.check(mArtNetEntity != nullptr, "unable to find Art-Net entity with name: %s", "ArtNetEntity"))
			return false;

		// All done!
		return true;
	}


	// Called when the window is updating
	void ArtNetReceiveApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Display general information
		showGeneralInfo();

		// Display received Art-Net
		showReceivedArtnet();
	}


	// Called when the window is going to render
	void ArtNetReceiveApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
	}


	void ArtNetReceiveApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void ArtNetReceiveApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// f is pressed, toggle full-screen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}


	int ArtNetReceiveApp::shutdown()
	{
		return 0;
	}


	void nap::ArtNetReceiveApp::showGeneralInfo()
	{
		ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Once);
		ImGui::Begin("Art-Net Receiving Demo");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::Separator();
		ImGui::TextWrapped("Use Napkin to change properties such as the Net, SubNet and Universe for receiving Art-Net.");
		ImGui::Separator();
		std::stringstream note;
		note << "Note:";
		note << "\n\nWhen using this demo together with the Art-Net controller (artnetsend demo), make sure to specify an IP Address for the Art-Net receiver.";
		note << "\n\nThe controller will always bind a socket to 0.0.0.0:6454, which is the Art-Net default. The receiver will bind to the IP Address and Port specified in the JSON file.";
		note << "\n\nThis prevents a conflict of two ports listening to the same local end-point";
		ImGui::TextWrapped(note.str().c_str());
		ImGui::End();
	}


	void nap::ArtNetReceiveApp::showReceivedArtnet()
	{
		auto artnet_handler = mArtNetEntity->findComponent<ArtNetHandlerComponentInstance>();
		auto received_data = artnet_handler->getData();

		int32_t offset = 0;
		int32_t total_count = static_cast<int32_t>(received_data.size());

		ImGui::SetNextWindowPos(ImVec2(384, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(480, 656), ImGuiCond_Once);
		ImGui::Begin("Received Art-Net");
		ImGui::Text("Universe: %d, Subnet: %d", artnet_handler->mInput->mUniverse, artnet_handler->mInput->mSubNet);
		ImGui::SliderInt("Channels per Group", &mArtNetInputGroupSize, 1, 512);
		ImGui::Separator();

		if (total_count == 0)
			ImGui::TextColored(ImVec4(mGuiService->getColors().mFront2Color), "No data");

		while (offset < total_count)
		{
			// The amount of channels to display in this histogram
			int32_t count = std::min(mArtNetInputGroupSize, total_count - offset);

			// The label for this histogram
			std::string label = count > 1 ?
				utility::stringFormat("Channels: %.3d to %.3d", offset + 1, offset + count) :
				utility::stringFormat("Channel: %.3d", offset + 1);

			// Plot the historgram
			ImGui::PlotHistogram(label.c_str(), received_data.data() + offset, count, 0, NULL, 0.0f, 255.0f, ImVec2(0, 64));

			// Increment the offset
			offset += count;
		}

		ImGui::End();
	}
}
