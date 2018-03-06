#pragma once

// Mod nap render includes
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <app.h>

// Audio includes
#include <audio/component/playbackcomponent.h>

namespace nap
{
	/**
	 * Demo application that is called from within the main loop
	 *
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
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window
        ObjectPtr<audio::AudioBufferResource> mBuffer = nullptr;        //< Pointer to the audio file in memory
        audio::PlaybackComponentInstance* mPlaybackComponent = nullptr; //< Component that takes care of audio playback
        audio::TimeValue mStartPosition = 0;
        audio::TimeValue mDuration = 0;
        audio::TimeValue mFadeInTime = 0;
        audio::TimeValue mFadeOutTime = 0;
        audio::ControllerValue mPitch = 1.0;
        audio::ControllerValue mPanning = 0.5;
        RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };    //< GUI text highlight color
	};
}
