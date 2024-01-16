/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "audiodevicesettingsgui.h"

// External Includes
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <app.h>

// Audio includes
#include <audio/component/playbackcomponent.h>
#include <audio/service/audioservice.h>

namespace nap
{
	using namespace rtti;

	/**
     * This demo application shows how to playback an audio file using audio::PlaybackComponent.
     * It shows how to start and stop playback and how to modify playback parameters.
	 */
	class AudioPlaybackApp : public App
	{
		RTTI_ENABLE(App)
	public:
		AudioPlaybackApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
        audio::PortAudioService* mAudioService = nullptr;               //< Manages audio devices and processing
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window
        ObjectPtr<audio::AudioBufferResource> mBuffer = nullptr;        //< Pointer to the audio file in memory
        ObjectPtr<nap::EntityInstance> mAudioEntity = nullptr;
        audio::TimeValue mStartPosition = 0;                            //< Start position of the playback in ms
        audio::TimeValue mDuration = 0;                                 //< Duration of the playback in ms
        audio::TimeValue mFadeInTime = 0;                               //< Fade in time in ms
        audio::TimeValue mFadeOutTime = 0;                              //< Fade out time in ms
        audio::ControllerValue mPitch = 1.0;                            //< Pitch of the playback in relation to original pitch of the audio file
        audio::ControllerValue mPanning = 0.5;                          //< Panning of the audio in the stereo field

        std::unique_ptr<audio::AudioDeviceSettingsGui> mAudioDeviceSettingsGui = nullptr; //< Gui to select audio device settings at runtime
	};
}
