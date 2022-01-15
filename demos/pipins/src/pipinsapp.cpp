// Local Includes
#include "pipinsapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>

namespace nap
{
    bool CoreApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
        mGpioService    = getCore().getService<nap::pipins::GpioService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("ParameterWindow");
		if (!error.check(mRenderWindow != nullptr, "unable to find parameter window with name: %s", "ParameterWindow"))
			return false;

        // Get the sequencer window
        mSequencerWindow = mResourceManager->findObject<nap::RenderWindow>("SequencerWindow");
        if (!error.check(mSequencerWindow != nullptr, "unable to find sequencer window with name: %s", "SequencerWindow"))
            return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

        mGpioPinPwm = mResourceManager->findObject<pipins::GpioPin>("GpioPinPwm");
        if (!error.check(mGpioPinPwm != nullptr, "unable to find gpio pwm pin with name: %s", "GpioPinPwm"))
            return false;

        mGpioPin = mResourceManager->findObject<pipins::GpioPin>("GpioPin");
        if (!error.check(mGpioPin != nullptr, "unable to find gpio digital pin with name: %s", "GpioPin"))
            return false;

        mParameterPwm = mResourceManager->findObject<ParameterInt>("PulseWidthParameter");
        if (!error.check(mParameterPwm != nullptr, "unable to find parameter with name: %s", "PulseWidthParameter"))
            return false;

        mParameterBlink = mResourceManager->findObject<ParameterFloat>("BlinkParameter");
        if (!error.check(mParameterBlink != nullptr, "unable to find parameter with name: %s", "BlinkParameter"))
            return false;

        mSequencerEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
        if (!error.check(mSequencerEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
            return false;

        mGpioService->setPwmRange(1024);
        mParameterPwm->valueChanged.connect([this](const int &value)
        {
            mGpioPinPwm->setPwmValue(value);
        });

        mParameterBlink->valueChanged.connect([this](const float &value)
        {
            mGpioPin->setDigitalWrite(value > 0 ? pipins::EPinValue::HIGH : pipins::EPinValue::LOW);
        });

		// All done!
        return true;
    }


    // Called when the window is going to render
    void CoreApp::render()
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

            mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

        // Begin recording the render commands for the main render window
        if (mRenderService->beginRecording(*mSequencerWindow))
        {
            // Begin render pass
            mSequencerWindow->beginRendering();

            mGuiService->draw();

            // Stop render pass
            mSequencerWindow->endRendering();

            // End recording
            mRenderService->endRecording();
        }

		// Proceed to next frame
		mRenderService->endFrame();
    }


    void CoreApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void CoreApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}
		mInputService->addEvent(std::move(inputEvent));
    }


    int CoreApp::shutdown()
    {
		return 0;
    }


    void CoreApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the scene
		// This is explicit because we don't know what entity should handle the events from a specific window.
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

        mGuiService->selectWindow(mRenderWindow.get());

        ImGui::Begin("GPIO");

        int pwm = mParameterPwm->mValue;
        if(ImGui::SliderInt("PWM Value", &pwm, 0, 1024))
        {
            mParameterPwm->setValue(pwm);
        }

        bool blink = mParameterBlink->mValue > 0;
        if(ImGui::Checkbox("Blink", &blink))
        {
            mParameterBlink->setValue(blink ? 1 : 0);
        }

        ImGui::End();

        mGuiService->selectWindow(mSequencerWindow.get());
        mSequencerEditorGUI->show();
    }
}
