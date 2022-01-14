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
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

        mGpioPinPwm = mResourceManager->findObject<pipins::GpioPin>("GpioPinPwm");
        if (!error.check(mGpioPinPwm != nullptr, "unable to find gpio pwm pin with name: %s", "GpioPinPwm"))
            return false;

        mGpioPinPwm->setPwmValue(mPwmValue);

        mGpioPin = mResourceManager->findObject<pipins::GpioPin>("GpioPin");
        if (!error.check(mGpioPin != nullptr, "unable to find gpio digital pin with name: %s", "GpioPin"))
            return false;

        mGpioPin->setDigitalWrite(mBlink ? pipins::EPinValue::HIGH : pipins::EPinValue::LOW);

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

			// Draw GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

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

        ImGui::Begin("GPIO");

        if(ImGui::SliderInt("PWM Value", &mPwmValue, 0, 1024))
        {
            mGpioPinPwm->setPwmValue(mPwmValue);
        }

        if(ImGui::Checkbox("Blink", &mBlink))
        {
            mGpioPin->setDigitalWrite(mBlink ? pipins::EPinValue::HIGH : pipins::LOW);
        }

        ImGui::End();
    }
}
