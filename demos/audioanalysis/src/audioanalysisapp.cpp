#include "audioanalysisapp.h"

// Nap includes
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

// Audio includes
#include <audio/component/playbackcomponent.h>
#include <audio/component/inputcomponent.h>
#include <audio/component/outputcomponent.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioAnalysisApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool AudioAnalysisApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("audioanalysis.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Find the world and camera entities
        ResourcePtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

        // Find audio entities and the level meter component that will perform the audio analysis
        mAudioEntity = scene->findEntity("Audio");
        auto levelMeter = mAudioEntity->findComponent<audio::LevelMeterComponentInstance>();
        
        // Set parameters for level meter component
        levelMeter->setCenterFrequency(mAnalysisFrequency);
        levelMeter->setBandWidth(mAnalysisBand);
        levelMeter->setFilterGain(mAnalysisGain);
        
        // Resize the vector containing the results of the analysis
        mPlotvalues.resize(128, 0);

		// Select render window
		mGuiService->selectWindow(mRenderWindow);

		return true;
	}
	
	
	/**
	 */
	void AudioAnalysisApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
        auto levelMeter = mAudioEntity->findComponent<audio::LevelMeterComponentInstance>();
        auto output = mAudioEntity->findComponent<audio::OutputComponentInstance>();
        auto input = mAudioEntity->findComponent<audio::AudioInputComponentInstance>();
        auto player = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
        
        assert(levelMeter);
        assert(input);
        assert(player);

		// Store new value in array
		mPlotvalues[mTickIdx] = levelMeter->getLevel();	// save new value so it can be subtracted later		
		if (++mTickIdx == mPlotvalues.size())			// increment current sample index
			mTickIdx = 0;

		// Draw some gui elements
		ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
		ImGui::Begin("Audio analysis");
        ImGui::PlotHistogram("", mPlotvalues.data(), mPlotvalues.size(), mTickIdx, nullptr, 0.0f, 0.2f, ImVec2(512, 128)); // Plot the output values
        ImGui::SliderFloat("Filter Frequency", &mAnalysisFrequency, 0.0f, 10000.0f, "%.3f", 2.0f);
        ImGui::SliderFloat("Filter Bandwidth", &mAnalysisBand, 1.f, 10000.0f, "%.3f", 2.0f);
        ImGui::SliderFloat("Audio Gain", &mAnalysisGain, 0.f, 10.0f, "%.3f", 1.0f);
        if (ImGui::RadioButton("Audio file input", mInputSource == EAudioFile))
            mInputSource = EAudioFile;
        if (ImGui::RadioButton("Audio device input", mInputSource == EAudioDevice))
            mInputSource = EAudioDevice;	
        ImGui::TextDisabled("Music: Hang by Breek (www.breek.me)");
		ImGui::End();
        
        if (mInputSource != mCurrentInputSource)
        {
            mCurrentInputSource = mInputSource;
            switch (mCurrentInputSource) {
                case EAudioFile:
                    levelMeter->setInput(*player);
                    output->setInput(*player);
                    break;
                    
                case EAudioDevice:
                    levelMeter->setInput(*input);
                    output->setInput(*input);
                    break;

                default:
                    break;
            }
        }
        
        // Update the audio level meter analysis component
        levelMeter->setCenterFrequency(mAnalysisFrequency);
        levelMeter->setBandWidth(mAnalysisBand);
        levelMeter->setFilterGain(mAnalysisGain);
	}

	
	/**
	 */
	void AudioAnalysisApp::render()
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

			// Draw our gui
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
	void AudioAnalysisApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void AudioAnalysisApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	void AudioAnalysisApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}


	int AudioAnalysisApp::shutdown()
	{
		return 0;
	}
}
