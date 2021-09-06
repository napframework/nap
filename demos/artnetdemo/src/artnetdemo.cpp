/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "artnetdemo.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <artnethandler.h>
#include <algorithm>
#include <sstream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtNetDemo)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool ArtNetDemo::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
		mResourceManager = getCore().getResourceManager();

		// Convert our path and load resources from file
		if (!mResourceManager->loadFile("artnetdemo.json", error))
			return false;

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

		// Get the Art-Net controller
		mArtNetController = mResourceManager->findObject<ArtNetController>("ArtNetController");
		if (!error.check(mArtNetController != nullptr, "unable to find Art-Net controller with name: %s", "ArtNetController"))
			return false;

		// Allocate memory for Art-Net output channels
		mArtNetOutputChannels.resize(mArtNetOutputChannelCount);

		// All done!
		return true;
	}


	// Called when the window is updating
	void ArtNetDemo::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Display general information
		showGeneralInfo();

		// Display received Art-Net
		showReceivedArtnet();

		// Display UI for sending Art-Net
		showSendArtnet();
	}


	// Called when the window is going to render
	void ArtNetDemo::render()
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


	void ArtNetDemo::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	void ArtNetDemo::inputMessageReceived(InputEventPtr inputEvent)
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


	int ArtNetDemo::shutdown()
	{
		return 0;
	}


	void nap::ArtNetDemo::showGeneralInfo()
	{
		ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(256, 128), ImGuiCond_Once);
		ImGui::Begin("Art-Net Demo");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();
	}


	void nap::ArtNetDemo::showReceivedArtnet()
	{
		auto artnet_handler = mArtNetEntity->findComponent<ArtNetHandlerComponentInstance>();
		auto received_data = artnet_handler->getData();

		int32_t offset = 0;
		int32_t total_count = static_cast<int32_t>(received_data.size());

		ImGui::SetNextWindowPos(ImVec2(320, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(448, 656), ImGuiCond_Once);
		ImGui::Begin("Received Art-Net");
		ImGui::SliderInt("Channels per Group", &mArtNetInputGroupSize, 1, 512);

		while (offset < total_count)
		{
			// The amount of channels to display in this histogram
			int32_t count = std::min(mArtNetInputGroupSize, total_count - offset);

			// The label for this histogram
			std::stringstream label;
			if (count > 1)
				label << "Channels " << (offset + 1) << " to " << (offset + count);
			else
				label << "Channel " << (offset + 1);

			// Plot the historgram
			ImGui::PlotHistogram(label.str().c_str(), received_data.data() + offset, count, 0, NULL, 0.0f, 255.0f, ImVec2(0, 64));

			// Increment the offset
			offset += count;
		}

		ImGui::End();
	}


	void nap::ArtNetDemo::showSendArtnet()
	{
		ImGui::SetNextWindowPos(ImVec2(800, 32), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(448, 656), ImGuiCond_Once);
		ImGui::Begin("Send Art-Net");

		// Define the amount of channels to send
		if (ImGui::SliderInt("Channel Count", &mArtNetOutputChannelCount, 1, 512))
			mArtNetOutputChannels.resize(mArtNetOutputChannelCount);

		// Fill the output channels with values
		for (int16_t i = 0; i < mArtNetOutputChannelCount; i++)
		{
			std::stringstream label;
			label << "Channel " << (i + 1);
			ImGui::SliderInt(label.str().c_str(), &mArtNetOutputChannels[i], 0, 255);
			mArtNetController->send(static_cast<uint8_t>(mArtNetOutputChannels[i]), i);
		}

		ImGui::End();
	}
}
