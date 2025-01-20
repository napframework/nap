/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiovisualfftapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <perspcameracomponent.h>
#include <computecomponent.h>
#include <parametergui.h>
#include <depthsorter.h>
#include <renderdofcomponent.h>
#include <rendertotexturecomponent.h>
#include <audio/component/playbackcomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioVisualFFTApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to NAP
	 */
	bool AudioVisualFFTApp::init(utility::ErrorState& errorState)
	{
		// Retrieve services
		mRenderService			= getCore().getService<RenderService>();
		mRenderAdvancedService	= getCore().getService<RenderAdvancedService>();
		mSceneService			= getCore().getService<SceneService>();
		mInputService			= getCore().getService<InputService>();
		mGuiService				= getCore().getService<IMGuiService>();

		// Fetch the resource manager
		mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");
		if (!errorState.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the render target
		mRenderTarget = mResourceManager->findObject<RenderTarget>("RenderTarget");
		if (!errorState.check(mRenderTarget != nullptr, "unable to find render target with name: %s", "RenderTarget"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!errorState.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

		// Get the camera entity
		mCameraEntity = mScene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "unable to find entity with name: %s", "CameraEntity"))
			return false;

		mRenderCameraEntity = mScene->findEntity("RenderCameraEntity");
		if (!errorState.check(mRenderCameraEntity != nullptr, "unable to find entity with name: %s", "RenderCameraEntity"))
			return false;

		mRenderEntity = mScene->findEntity("RenderEntity");
		if (!errorState.check(mRenderEntity != nullptr, "unable to find entity with name: %s", "RenderEntity"))
			return false;

		mWorldEntity = mScene->findEntity("WorldEntity");
		if (!errorState.check(mWorldEntity != nullptr, "unable to find entity with name: %s", "WorldEntity"))
			return false;

		// Cache mask
		mLitRenderMask = mRenderService->getRenderMask("Lit");

		// Pre-render cube map
		if (!preRenderCubeMap(errorState))
			return false;

		// Connect reload slot
		mResourceManager->mPostResourcesLoadedSignal.connect(mReloadSlot);

		// Start audio file playback
		auto audioEntity = mScene->findEntity("AudioEntity");
		if (!errorState.check(audioEntity != nullptr, "unable to find entity with name: %s", "AudioEntity"))
			return false;
		auto playbackComponent = audioEntity->findComponent<audio::PlaybackComponentInstance>();
		if (!errorState.check(playbackComponent != nullptr, "unable to find PlaybackComponentInstance"))
			return false;
		playbackComponent->start();

		// All done!
		return true;
	}


	bool AudioVisualFFTApp::preRenderCubeMap(utility::ErrorState& errorState)
	{
		auto cube_map = mResourceManager->findObject<CubeRenderTarget>("CubeRenderTarget");
		if (!errorState.check(cube_map != nullptr, "unable to find cube render target with name: %s", "CubeRenderTarget"))
			return false;

		// Filter components to render into the cube map with the environment mask
		std::vector<RenderableComponentInstance*> comps;
		mScene->getRootEntity().getComponentsOfTypeRecursive(comps);
		auto env_mask = mRenderService->getRenderMask("Environment");
		auto env_comps = mRenderService->filterObjects(comps, env_mask);

		// Queue a headless render command to carry out next time beginHeadlessRecording() is called
		mRenderService->queueHeadlessCommand([cube=cube_map, comps=env_comps](RenderService& renderService)
		{
			// Render the environment components into the cube map
			cube->render([rs=&renderService, comps=comps](CubeRenderTarget& target, const glm::mat4& projection, const glm::mat4& view) {
				rs->renderObjects(target, projection, view, { comps }, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
			});
		});
		return true;
	}


	void AudioVisualFFTApp::reload()
	{
		utility::ErrorState error_state;
		if (!preRenderCubeMap(error_state))
			nap::Logger::error(error_state.toString());
	}


	// Update app
	void AudioVisualFFTApp::update(double deltaTime)
	{
		// Use a default input router to forward input events (recursively) to all input components in the default scene
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

		// GUI
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::Text(utility::stringFormat("Frametime: %.02fms", deltaTime * 1000.0).c_str());

		for (const auto& gui : mResourceManager->getObjects<ParameterGUI>())
			gui->show(false);

		ImGui::Text("Music: Hang by Breek (www.breek.me)");
		ImGui::End();
	}
	
	
	// Render app
	void AudioVisualFFTApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Compute
		std::vector<ComputeComponentInstance*> compute_comps;
		mWorldEntity->getComponentsOfTypeRecursive<ComputeComponentInstance>(compute_comps);
		if (mRenderService->beginComputeRecording())
		{
			mRenderService->computeObjects(compute_comps);
			mRenderService->endComputeRecording();
		}

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Headless
		if (mRenderService->beginHeadlessRecording())
		{
			// Offscreen color pass -> Render all available geometry to the color texture bound to the render target.
			mRenderTarget->beginRendering();
			utility::ErrorState error_state;
			auto lit_comps = mRenderService->filterObjects(render_comps, mLitRenderMask);
			mRenderAdvancedService->pushLights(lit_comps);

			auto& cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
			mRenderService->renderObjects(*mRenderTarget, cam, render_comps);
			mRenderTarget->endRendering();

			// DOF
			mRenderEntity->getComponent<RenderDOFComponentInstance>().draw();
		}
		mRenderService->endHeadlessRecording();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get camera to render with
			auto& cam = mRenderCameraEntity->getComponent<CameraComponentInstance>();

			// Get render chroma component responsible for rendering final texture
			auto* chroma_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("RenderChroma");
			assert(chroma_comp != nullptr);

			// Render composite component
			// The nap::RenderToTextureComponentInstance transforms a plane to match the window dimensions and applies the texture to it.
			mRenderService->renderObjects(*mRenderWindow, cam, { chroma_comp });
		
			// GUI
			if (!mHideGUI)
				mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
	}
	

	void AudioVisualFFTApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void AudioVisualFFTApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// If we pressed escape, quit the loop
			auto* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// f is pressed, toggle full-screen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();

			// h is pressed, toggle GUI
			if (press_event->mKey == nap::EKeyCode::KEY_h)
				mHideGUI = !mHideGUI;
		}
		// Add event, so it can be forwarded on update
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int AudioVisualFFTApp::shutdown()
	{
		return 0;
	}

}
