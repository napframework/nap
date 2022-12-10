/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audioplaybackapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <triangleiterator.h>
#include <meshutils.h>
#include <mathutils.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioPlaybackApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool AudioPlaybackApp::init(utility::ErrorState& error)
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
        mBuffer = mResourceManager->findObject<audio::AudioBufferResource>("audioFile");

		// Find the audio entity
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
        mAudioEntity = scene->findEntity("audioEntity");
        
        // Find the audio playback component and initialize parameters
        auto playbackComponent = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
		mFadeInTime = playbackComponent->getFadeInTime() / 1000.0f;
        mFadeOutTime = playbackComponent->getFadeOutTime() / 1000.0f;
        mPitch = playbackComponent->getPitch();
        mPanning = playbackComponent->getStereoPanning();

        mAudioDeviceSettingsGui = std::make_unique<audio::AudioDeviceSettingsGui>(*getCore().getService<audio::AudioService>(), false);

		return true;
	}


	/**
	 */
	void AudioPlaybackApp::update(double deltaTime)
	{
        auto playbackComponent = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
        
		// Draw some gui elements to control audio playback
		ImGui::Begin("Audio Playback", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (!playbackComponent->isPlaying())
        {
            if (ImGui::Button("Play"))
                playbackComponent->start(mStartPosition * 1000.0f, mDuration * 1000.0f);
        }
        else
		{
            if (ImGui::Button("Stop"))
                playbackComponent->stop();
        }

		float length_seconds = mBuffer->getSize() / (mBuffer->getSampleRate() / 1000.0f) / 1000.0f;
		ImGui::SliderFloat("Start Position (s)", &mStartPosition, 0, length_seconds, "%.3f", 2);
		ImGui::SliderFloat("Duration (0 = until end)", &mDuration, 0, 10.0f, "%.3f", 2);
		if (ImGui::SliderFloat("Fade In (s)", &mFadeInTime, 0, 2.0f, "%.3f", 2))
		{
			playbackComponent->setFadeInTime(mFadeInTime * 1000.0f);
		}
		if (ImGui::SliderFloat("Fade Out (s)", &mFadeOutTime, 0, 2.0f, "%.3f", 2))
		{
			playbackComponent->setFadeOutTime(mFadeOutTime * 1000.0f);
		}
		if (ImGui::SliderFloat("Pitch", &mPitch, 0.5, 2, "%.3f", 1))
		{
			playbackComponent->setPitch(mPitch);
		}
		if (ImGui::SliderFloat("Panning", &mPanning, 0.f, 1.f, "%.3f", 1))
		{
			playbackComponent->setStereoPanning(mPanning);
		}

		// Show audio device settings
        if (ImGui::CollapsingHeader("Driver Settings"))
        {
            mAudioDeviceSettingsGui->drawGui();

            // Save audio device settings to config file
            if (ImGui::Button("Save"))
            {
                utility::ErrorState errorState;
                auto configPath = getCore().getProjectInfo()->mServiceConfigFilename;
                if (configPath.empty())
                    configPath = "config.json";
                if (!getCore().writeConfigFile(configPath, errorState))
                    Logger::warn("Failed to write config file: %s", errorState.toString().c_str());
            }
        }

        ImGui::Text("Music: Hang by Breek (www.breek.me)");
        ImGui::End();
	}

	
	/**
	 */
	void AudioPlaybackApp::render()
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
	void AudioPlaybackApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void AudioPlaybackApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int AudioPlaybackApp::shutdown()
	{
		return 0;
	}
}
