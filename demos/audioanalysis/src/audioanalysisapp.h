/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <app.h>
#include <spheremesh.h>

// Audio includes
#include <audio/component/levelmetercomponent.h>

namespace nap
{
	/**
	 * Demo application that shows how to analyze an audio signal in order to extract the output level of a certain frequency band of the signal.
     * Audio is generated using an audio::PlaybackComponent or an audio::AudioInputComponent. The signal is sent to an audio::OutpuComponent so we can hear it and to an audio::LevelMeterComponent that we can adjust with a GUI.
     * The output of the LevelMeterComponent is plotted in the GUI.
	 */
	class AudioAnalysisApp : public App
	{
		RTTI_ENABLE(App)
	public:
		AudioAnalysisApp(nap::Core& core) : App(core)	{ }

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
		 *	Toggles full screen
		 */
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);
		
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
		ResourcePtr<RenderWindow> mRenderWindow;						//< Pointer to the render window
        rtti::ObjectPtr<EntityInstance> mAudioEntity = nullptr;         //< Entity that contains the audio processing
        audio::ControllerValue mAnalysisFrequency = 500.f;              //< Center frequency of the analysis
        audio::ControllerValue mAnalysisBand = 100.f;                   //< Bandwidth of the analysis
        audio::ControllerValue mAnalysisGain = 1.0f;                    //< Factor to gain the analysis input before analyzing
		std::array<audio::ControllerValue, 256> mPlotvalues = {};		//< Output of the analysis will be stored chronologically in this factor, so we can draw a plot of the data
		nap::SteadyTimer mTimer;										//< Timer
		uint32 mTickSum = 0;
		uint32 mTickIdx = 0;

        enum InputSource {
            EAudioDevice, EAudioFile
        };
        InputSource mCurrentInputSource = EAudioFile;                   //< Indicates wether input from the audio device or a file will be analyzed
        InputSource mInputSource = EAudioFile;                          //< GUI helper variable for the user to select a new input 
	};
}
