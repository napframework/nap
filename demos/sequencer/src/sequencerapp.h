#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <parameterservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <app.h>
#include <sequenceeditorgui.h>
#include <parametergui.h>

namespace nap 
{
	using namespace rtti;

    /**
     * Main application called from within the main loop
     */
    class SequencerApp : public App 
	{
    public:

		/**
		 * Constructor
		 */
        SequencerApp(nap::Core& core) : App(core) {}

        /**
         * Provide the app with the json file that will be loaded, must be called before initialization.
		 * @param fileName the json file to load on initialization
         */
        void setFilename(const std::string& fileName)								{ mFilename = fileName; }

        /**
         * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
         */
        bool init(utility::ErrorState& error) override;

		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

        /**
         * Render is called after update. Use this call to render objects to a specific target
         */
        void render() override;

        /**
         * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
         */
        void windowMessageReceived(WindowEventPtr windowEvent) override;

        /**
         * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
         */
        void inputMessageReceived(InputEventPtr inputEvent) override;

        /**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
         */
        int shutdown() override;

    private:
        ResourceManager*		mResourceManager = nullptr;				///< Manages all the loaded data
        std::string				mFilename = "";							///< The JSON file that is loaded on initialization
		RenderService*			mRenderService = nullptr;				///< Render Service that handles render calls
		SceneService*			mSceneService = nullptr;				///< Manages all the objects in the scene
		InputService*			mInputService = nullptr;				///< Input service for processing input
		IMGuiService*			mGuiService = nullptr;					///< Manages GUI related update / draw calls
		ParameterService*		mParameterService = nullptr;			///< Manages all the parameters
		ObjectPtr<RenderWindow> mParameterWindow = nullptr;				///< Pointer to the parameter render window	
		ObjectPtr<RenderWindow> mTimelineWindow = nullptr;				///< Pointer to the timeline render window
		ObjectPtr<Scene>		mScene = nullptr;						///< Pointer to the main scene
		
		std::unique_ptr<ParameterGUI> mParameterGUI = nullptr;			///< Displays the parameters
		ObjectPtr<SequenceEditorGUI> mSequenceEditorGUI = nullptr;		///< Displays the sequence editor gui
		ObjectPtr<ParameterGroup> mParameterGroup = nullptr;			///< Link to parameters
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	///< GUI text highlight color
	};
}
