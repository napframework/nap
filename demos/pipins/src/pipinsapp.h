/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <imguiservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <app.h>
#include <gpiopin.h>
#include <parameternumeric.h>
#include <sequenceeditorgui.h>

namespace nap
{
	using namespace rtti;

    /**
     * Raspberry PI GPIO example application
	 *
	 * This application is an example of how to use the GPIO of the raspberry pi using mod_nappipins in conjunction
     * with a SequencePlayer
     * This application controls pulse width modulation on one GPIO pin and uses another pin as a simple on/off blink
     * example. See the in "default.json" declared GPIO pin numbers.
     * Run the example as sudo, since using PWM_OUTPUT mode on GPIO pin requires root access.
     */
    class CoreApp : public App
	{
    public:
		/**
		 * Constructor
		 */
        CoreApp(nap::Core& core) : App(core) {}

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
        /**
         * Draws application GUI
         */
        void drawGui();

        /**
         * Callback hookup to onPostResourcesLoaded signal of the resource manager
         * Hooks up parameters to GPIO pins and starts SequencePlayer
         */
        void onPostResourcesLoaded();

        ResourceManager*			mResourceManager = nullptr;		///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;		///< Render Service that handles render calls
		InputService*				mInputService = nullptr;		///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;			///< Manages GUI related update / draw calls
        pipins::GpioService*        mGpioService = nullptr;         ///< Manages GPIO related calls
		ObjectPtr<RenderWindow>		mRenderWindow;					///< Pointer to the render window
		ObjectPtr<Scene>			mScene = nullptr;				///< Pointer to the main scene
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;		///< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mGnomonEntity = nullptr;		///< Pointer to the entity that can render the gnomon

        ResourcePtr<pipins::GpioPin>    mGpioPinPwm = nullptr;      ///< Pointer to GPIO pin controlling pulse width
        ResourcePtr<pipins::GpioPin>    mGpioPinBlink = nullptr;    ///< Pointer to GPIO pin for digitalWrite calls
        ResourcePtr<ParameterInt>       mParameterPwm = nullptr;    ///< Pointer to parameter being animated by GUI and Sequencer controlling pulse width
        ResourcePtr<ParameterFloat>     mParameterBlink = nullptr;  ///< Pointer to parameter being animated by GUI and Sequencer controlling blink
        ResourcePtr<SequenceEditorGUI>  mSequenceEditorGUI = nullptr; ///< Pointer to the gui of the Sequencer
        ResourcePtr<SequencePlayer>     mSequencePlayer = nullptr; ///< Pointer to SequencePlayer

        RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	///< GUI text highlight color
	};
}
