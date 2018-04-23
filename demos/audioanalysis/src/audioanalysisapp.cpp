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

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

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
        mAnalysisPlotValues.resize(100, 0);

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

        // Shift the values in the vector with output values one position to the right, in order to make place for a new value.
        for (auto i = mAnalysisPlotValues.size() - 1; i > 0; i--)
            mAnalysisPlotValues[i] = mAnalysisPlotValues[i - 1];
        // Insert the new output value at the top of the vector.
        mAnalysisPlotValues[0] = levelMeter->getLevel();
        
		// Draw some gui elements
		ImGui::Begin("Audio analysis");
        ImGui::PlotLines("", mAnalysisPlotValues.data(), mAnalysisPlotValues.size(), 0, nullptr, 0.0f, 0.2f, ImVec2(0, 200)); // Plot the output values
        ImGui::SliderFloat("Filter Frequency", &mAnalysisFrequency, 0.0f, 10000.0f, "%.3f", 2.0f);
        ImGui::SliderFloat("Filter Bandwidth", &mAnalysisBand, 1.f, 10000.0f, "%.3f", 2.0f);
        ImGui::SliderFloat("Audio Gain", &mAnalysisGain, 0.f, 10.0f, "%.3f", 1.0f);
        if (ImGui::RadioButton("Audio file input", mInputSource == EAudioFile))
            mInputSource = EAudioFile;
        if (ImGui::RadioButton("Audio device input", mInputSource == EAudioDevice))
            mInputSource = EAudioDevice;
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
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Draw our gui
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
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
