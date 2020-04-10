// Local Includes
#include "timeline.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>


namespace nap 
{    
    bool CoreApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();
		
		// Fetch the scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		// Convert our path and load resources from file
        auto abspath = utility::getAbsolutePath(mFilename);
        nap::Logger::info("Loading: %s", abspath.c_str());
        if (!mResourceManager->loadFile(mFilename, error))
            return false;

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		mSequenceEditorGUI = mResourceManager->findObject<SequenceEditorGUI>("SequenceEditorGUI");
		if (!error.check(mSequenceEditorGUI != nullptr, "unable to find SequenceEditorGUI with name: %s", "SequenceEditorGUI"))
			return false;

		mParameterGroup = mResourceManager->findObject<ParameterGroup>("ParameterGroup");
		if (!error.check(mParameterGroup != nullptr, "unable to find ParameterGroup with name: %s", "ParameterGroup"))
			return false;

		// All done!
        return true;
    }


    // Called when the window is going to render
	void CoreApp::render()
	{
		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		ImGui::Begin("Parameters");

		for (const auto& parameterResource : mParameterGroup->mParameters)
		{
			if (parameterResource->get_type().is_derived_from<ParameterFloat>())
			{
				ParameterFloat* parameter = static_cast<ParameterFloat*>(parameterResource.get());

				std::string name = parameter->getDisplayName();

				float value = parameter->mValue;
				if (ImGui::SliderFloat(name.c_str(),
					&value,
					parameter->mMinimum,
					parameter->mMaximum))
				{
					parameter->setValue(value);
				}
			}
			else if (parameterResource->get_type().is_derived_from<ParameterInt>())
			{
				ParameterInt* parameter = static_cast<ParameterInt*>(parameterResource.get());

				std::string name = parameter->getDisplayName();

				int value = parameter->mValue;
				if (ImGui::SliderInt(name.c_str(),
					&value,
					parameter->mMinimum,
					parameter->mMaximum))
				{
					parameter->setValue(value);
				}
			}
			else if (parameterResource->get_type().is_derived_from<ParameterDouble>())
			{
				ParameterDouble* parameter = static_cast<ParameterDouble*>(parameterResource.get());

				std::string name = parameter->getDisplayName();

				float value = static_cast<float>(parameter->mValue);
				if (ImGui::SliderFloat(name.c_str(),
					&value,
					parameter->mMinimum,
					parameter->mMaximum))
				{
					parameter->setValue(value);
				}
			}
			else if (parameterResource->get_type().is_derived_from<ParameterLong>())
			{
				ParameterLong* parameter = static_cast<ParameterLong*>(parameterResource.get());

				std::string name = parameter->getDisplayName();

				int value = static_cast<int>(parameter->mValue);
				if (ImGui::SliderInt(name.c_str(),
					&value,
					parameter->mMinimum,
					parameter->mMaximum))
				{
					parameter->setValue(value);
				}
			}

		}
		
		ImGui::End();
			

		// Draw sequence editor gui
		mSequenceEditorGUI->draw();

		// Draw our gui
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
    }


    void CoreApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void CoreApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		mInputService->addEvent(std::move(inputEvent));
    }


    int CoreApp::shutdown()
    {
		return 0;
    }


    void CoreApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });
    }
}
