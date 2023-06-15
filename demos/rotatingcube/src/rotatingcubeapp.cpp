#include "rotatingcubeapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <rotatecomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RotatingCubeApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize the application.
	 * Fetch all required resources and entities,
	 */
	bool RotatingCubeApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

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

		// Get the camera entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		if (!error.check(mCameraEntity != nullptr, "unable to find entity with name: %s", "CameraEntity"))
			return false;

		// Get the Gnomon entity
		mGnomonEntity = mScene->findEntity("GnomonEntity");
		if (!error.check(mGnomonEntity != nullptr, "unable to find entity with name: %s", "GnomonEntity"))
			return false;

		// Find the cube entity
		mCubeEntity = mScene->findEntity("CubeEntity");
		if (!error.check(mCubeEntity != nullptr, "unable to find entity with name: %s", "CubeEntity"))
			return false;

		// All done!
		return true;
	}
	
	
	// Update the app
	void RotatingCubeApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// Setup cube controls
		ImGui::Begin("Cube Controls");
		auto& rotate_component = mCubeEntity->getComponent<nap::RotateComponentInstance>();
		ImGui::SliderFloat("Rotation Speed", &rotate_component.mProperties.mSpeed, 0.0f, 1.0f);
		ImGui::End();

	}
	
	
	// Render the cube & gui
	void RotatingCubeApp::render()
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

			// Get Perspective camera to render with
			auto& perp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();

			// Add cube to render
			std::vector<nap::RenderableComponentInstance*> components_to_render
			{
				&mCubeEntity->getComponent<RenderableComponentInstance>()
			};

			// Render cube
			mRenderService->renderObjects(*mRenderWindow, perp_cam, components_to_render);

			// Render GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
	}
	

	void RotatingCubeApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void RotatingCubeApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// f is pressed, toggle full-screen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int RotatingCubeApp::shutdown()
	{
		return 0;
	}

}
