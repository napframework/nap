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
    static bool drawControls = true;
    static bool drawSequencer = true;

    bool CoreApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
        mGpioService    = getCore().getService<nap::pipins::GpioService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

        // Get GPIO pin controlling pulse width
        mGpioPinPwm = mResourceManager->findObject<pipins::GpioPin>("GpioPinPwm");
        if (!error.check(mGpioPinPwm != nullptr, "unable to find gpio pwm pin with name: %s", "GpioPinPwm"))
            return false;

        // Get GPIO pin blink
        mGpioPinBlink = mResourceManager->findObject<pipins::GpioPin>("GpioPinBlink");
        if (!error.check(mGpioPinBlink != nullptr, "unable to find gpio blink pin with name: %s", "GpioPinBlink"))
            return false;

        // Get parameter controlling pulse width
        mParameterPwm = mResourceManager->findObject<ParameterInt>("PulseWidthParameter");
        if (!error.check(mParameterPwm != nullptr, "unable to find parameter with name: %s", "PulseWidthParameter"))
            return false;

        // Get parameter controlling blink
        mParameterBlink = mResourceManager->findObject<ParameterFloat>("BlinkParameter");
        if (!error.check(mParameterBlink != nullptr, "unable to find parameter with name: %s", "BlinkParameter"))
            return false;

        // Get SequenceEditor gui
        mSequenceEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
        if (!error.check(mSequenceEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
            return false;

        // Get SequencePlayer
        mSequencePlayer = mResourceManager->findObject<SequencePlayer>("SequencePlayer");
        if (!error.check(mSequencePlayer != nullptr, "unable to find SequencePlayer with name: %s", "SequencePlayer"))
            return false;

        // when resources get loaded, hook up callbacks to correct parameters and gpio pins
        mResourceManager->mPostResourcesLoadedSignal.connect([this](){ onPostResourcesLoaded(); });
        onPostResourcesLoaded();

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

        drawGui();
    }


    void CoreApp::drawGui()
    {
        mGuiService->selectWindow(mRenderWindow.get());

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Controls", nullptr, &drawControls);
                ImGui::MenuItem("Sequencer", nullptr, &drawSequencer);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if(drawControls)
        {
            ImGui::Begin("Controls");

            ImGui::Text(getCurrentDateTime().toString().c_str());
            RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
            ImGui::TextColored(clr, utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

            if(ImGui::CollapsingHeader("GPIO"))
            {
                int pwm = mParameterPwm->mValue;
                if(ImGui::SliderInt("PWM Value", &pwm, 0, 1024))
                {
                    if(!mSequencePlayer->getIsPlaying())
                        mParameterPwm->setValue(pwm);
                }

                bool blink = mParameterBlink->mValue > 0;
                if(ImGui::Checkbox("Blink", &blink))
                {
                    if(!mSequencePlayer->getIsPlaying())
                        mParameterBlink->setValue(blink ? pipins::EPinValue::HIGH : pipins::EPinValue::LOW);
                }
            }

            ImGui::End();
        }

        if(drawSequencer)
        {
            mSequenceEditorGUI->show();
        }
    }

    void CoreApp::onPostResourcesLoaded()
    {
        // hook up pins to parameter callbacks
        mParameterPwm->valueChanged.connect([this](const int &value)
        {
            mGpioPinPwm->setPwmValue(value);
        });
        mParameterBlink->valueChanged.connect([this](const float &value)
        {
            mGpioPinBlink->setDigitalWrite(value > 0 ? pipins::EPinValue::HIGH : pipins::EPinValue::LOW);
        });

        // fire up the sequence player and let it loop
        mSequencePlayer->setIsPaused(false);
        mSequencePlayer->setIsPlaying(true);
        mSequencePlayer->setIsLooping(true);
    }
}
