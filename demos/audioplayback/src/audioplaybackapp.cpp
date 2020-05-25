#include "audioplaybackapp.h"

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

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("audioplayback.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
        mBuffer = mResourceManager->findObject<audio::AudioBufferResource>("audioFile");

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

		// Find the audio entity
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
        mAudioEntity = scene->findEntity("audioEntity");
        
        // Find the audio playback component and initialize parameters
        auto playbackComponent = mAudioEntity->findComponent<audio::PlaybackComponentInstance>();
        mFadeInTime = playbackComponent->getFadeInTime();
        mFadeOutTime = playbackComponent->getFadeOutTime();
        mPitch = playbackComponent->getPitch();
        mPanning = playbackComponent->getStereoPanning();

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
                playbackComponent->start(mStartPosition, mDuration);
        }
        else {
            if (ImGui::Button("Stop"))
                playbackComponent->stop();
        }
        ImGui::SliderFloat("Start position", &mStartPosition, 0, mBuffer->getSize() / (mBuffer->getSampleRate() / 1000.), "%.3f", 2);
        ImGui::SliderFloat("Duration (0 is untill the end)", &mDuration, 0, 10000, "%.3f", 2);
        ImGui::SliderFloat("Fade In", &mFadeInTime, 0, 2000, "%.3f", 2);
        ImGui::SliderFloat("Fade Out", &mFadeOutTime, 0, 2000, "%.3f", 2);
        ImGui::SliderFloat("Pitch", &mPitch, 0.5, 2, "%.3f", 1);
        ImGui::SliderFloat("Panning", &mPanning, 0.f, 1.f, "%.3f", 1);
        ImGui::Text("Music: Hang by Breek (www.breek.me)");
		ImGui::End();
        
        // Apply GUI parameters to audio playback component
        playbackComponent->setFadeInTime(mFadeInTime);
        playbackComponent->setFadeOutTime(mFadeOutTime);
        playbackComponent->setPitch(mPitch);
        playbackComponent->setStereoPanning(mPanning);
	}

	
	/**
	 */
	void AudioPlaybackApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

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

	
	void AudioPlaybackApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}


	int AudioPlaybackApp::shutdown()
	{
	    utility::ErrorState errorState;
	    if (!getCore().writeConfigFile(errorState))
	        Logger::warn("Failed to write configuration file: %s", errorState.toString().c_str());
		return 0;
	}
}
