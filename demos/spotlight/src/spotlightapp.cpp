// Local Includes
#include "spotlightapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <renderablemeshcomponent.h>
#include <perspcameracomponent.h>
#include <imgui/imgui.h>
#include <spotlightcomponent.h>

namespace nap 
{    
    bool CoreApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
		mRenderAdvancedService = getCore().getService<nap::RenderAdvancedService>();

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

		// Get the camera and origin Gnomon entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		mSceneEntity = mScene->findEntity("SceneEntity");
		mSpotlightEntity = mScene->findEntity("SpotlightEntity");

		// Tags used to group and mask render objects
		mSceneTag = mResourceManager->findObject("SceneTag");
		mDebugTag = mResourceManager->findObject("DebugTag");

		// All done!
        return true;
    }


    // Render app
    void CoreApp::render()
    {
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Get all the possible objects to render
		std::vector<RenderableComponentInstance*> render_comps;
		mSceneEntity->getComponentsOfTypeRecursive<nap::RenderableComponentInstance>(render_comps);

		// Get frustrum to display
		auto& spotlight_comp = mSpotlightEntity->getComponent<SpotLightComponentInstance>();
		std::vector<RenderableComponentInstance*> frustrum_comp = {
			spotlight_comp.getFrustrum()
		};

		// Bake shadow maps
		if (mRenderService->beginHeadlessRecording())
		{
			mRenderAdvancedService->renderShadows(render_comps, true, *mSceneTag);
			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get Perspective camera to render with
			auto& perp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();

			// Render all the objects
			mRenderService->renderObjects(*mRenderWindow, perp_cam, render_comps, *mSceneTag);
			mRenderService->renderObjects(*mRenderWindow, perp_cam, render_comps, *mDebugTag);
			mRenderService->renderObjects(*mRenderWindow, perp_cam, frustrum_comp);

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


	// Update app
    void CoreApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the scene
		// This is explicit because we don't know what entity should handle the events from a specific window.
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// GUI
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::Text(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());
		ImGui::End();
    }
}
