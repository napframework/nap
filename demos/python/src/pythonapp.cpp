#include "pythonapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <pythonscriptcomponent.h>

// Register this application with RTTI, this is required by the AppRunner to
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PythonApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool PythonApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("python.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

        // Find the entity containing the PythonScriptComponent
        mPythonEntity = scene->findEntity("PythonEntity");
        if (mPythonEntity == nullptr)
        {
            error.fail("PythonEntity not found");
            return false;
        }
        auto pythonComponent = mPythonEntity->findComponent<PythonScriptComponentInstance>();
        if (!pythonComponent->get<float>("getValue", error, mValue)) // Initialize mValue using the getter in the python script.
            return false;

		return true;
	}


	/**
	 */
	void PythonApp::update(double deltaTime)
	{
        // Log some information to the top of the display
		ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        ImGui::Begin("Python demo");

        // The variable mValue is controlled by a slider.
        // The setter setValue() within the python script is called to set a member variable within python.
        // The getter getValue() is called to retrieve the python member's value and to display it/
        auto pythonComponent = mPythonEntity->findComponent<PythonScriptComponentInstance>();
        ImGui::Text("Call the PythonScriptComponent's setter using the slider:");
        ImGui::SliderFloat("", &mValue, 0.f, 1.f);
        utility::ErrorState errorState;
        if (!pythonComponent->call("setValue", errorState, mValue)) // Call setValue() method within the python script.
            nap::Logger::warn(errorState.toString());
        float returnValue = 0;
        if (!pythonComponent->get<float>("getValue", errorState, returnValue)) // Call getValue() method within the python script.
            nap::Logger::warn(errorState.toString());

        ImGui::Text(utility::stringFormat("The PythonScriptComponent's getter returns: %f", returnValue).c_str());

        ImGui::End();

	}


	/**
	 * Draw the gui + osc / midi messages to screen
	 */
	void PythonApp::render()
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
	void PythonApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void PythonApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int PythonApp::shutdown()
	{
		return 0;
	}


}
