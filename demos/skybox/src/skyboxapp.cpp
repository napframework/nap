/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "skyboxapp.h"

 // External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <renderskyboxcomponent.h>
#include <skyboxshader.h>
#include <blinnphongcolorshader.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SkyBoxApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool SkyBoxApp::init(utility::ErrorState& errorState)
	{
		// Create render service
		mRenderService			= getCore().getService<RenderService>();
		mRenderAdvancedService	= getCore().getService<RenderAdvancedService>();
		mInputService			= getCore().getService<InputService>();
		mSceneService			= getCore().getService<SceneService>();
		mGuiService				= getCore().getService<IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();

		// Gather resources
		auto scene = mResourceManager->findObject<Scene>("Scene");
		mRenderWindow = mResourceManager->findObject<RenderWindow>("RenderWindow");
		mDefaultInputRouter = scene->findEntity("DefaultInputRouterEntity");

		// Get world entity - parent of our renderable scene
		mWorldEntity = scene->findEntity("WorldEntity");
		if (!errorState.check(mWorldEntity != nullptr, "Missing WorldEntity"))
			return false;

		mCameraEntity = scene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing CameraEntity"))
			return false;

		// Cache cube maps
		mCubeMaps = mResourceManager->getObjects<CubeMapFromFile>();

		mGuiService->selectWindow(mRenderWindow);

		return true;
	}


	/**
	 * Forward all received input events to the input router.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our first person camera.
	 *
	 * We also set up our gui that is drawn at a later stage.
	 */
	void SkyBoxApp::update(double deltaTime)
	{
		// Update input
		DefaultInputRouter& input_router = mDefaultInputRouter->getComponent<DefaultInputRouterComponentInstance>().mInputRouter;
		{
			// Update input for first window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntity.get());

			Window* window = mRenderWindow.get();
			mInputService->processWindowEvents(*window, input_router, entities);
		}

		// Update GUI
		ImGui::Begin("Menu");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("%.02f fps | %.02f ms", getCore().getFramerate(), deltaTime*1000.0).c_str());

		// Here we create a simple combobox that allows the user to switch between cube maps
		// On selection, we must update the sampler instance of the sky box and all render components that display its reflection

		// Gather all skyboxes, there should be only one however
		std::vector<RenderSkyBoxComponentInstance*> skyboxes;
		mWorldEntity->getComponentsOfTypeRecursive<RenderSkyBoxComponentInstance>(skyboxes);
		if (!skyboxes.empty())
		{
			// Create a list of cubemap labels
			std::vector<const char*> labels;
			labels.reserve(mCubeMaps.size());
			std::for_each(mCubeMaps.begin(), mCubeMaps.end(), [&labels](const auto& cube) {
				labels.emplace_back(cube->mID.c_str());
			});

			static int item_index = 0;
			if (ImGui::Combo("CubeMaps", &item_index, labels.data(), mCubeMaps.size()))
			{
				// Update the sampler instance of the skybox
				auto* sampler = skyboxes.front()->getMaterialInstance().getOrCreateSampler<SamplerCubeInstance>(uniform::skybox::sampler::cubeTexture);
				sampler->setTexture(*mCubeMaps[item_index]);

				// Update the sampler isntances of all render components that display the reflection of the skybox
				std::vector<RenderableMeshComponentInstance*> render_comps;
				mWorldEntity->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(render_comps);
				for (auto& comp : render_comps)
				{
					auto* env_sampler = comp->getMaterialInstance().findSampler(uniform::blinnphongcolor::sampler::environmentMap);
					if (env_sampler != nullptr)
					{
						if (env_sampler->get_type().is_derived_from(RTTI_OF(SamplerCubeInstance)))
							static_cast<SamplerCubeInstance*>(env_sampler)->setTexture(*mCubeMaps[item_index]);
					}
				}
			}
		}
		ImGui::Dummy(ImVec2(0.0f, 5.0f * mGuiService->getScale()));

		// Display parameter GUI(s)
		const auto param_guis = mResourceManager->getObjects<ParameterGUI>();
		for (const auto& gui : param_guis)
		{
			if (ImGui::CollapsingHeader(gui->mID.c_str()))
				gui->show(false);
		}
		ImGui::End();
	}


	/**
	 * Render all objects to screen
	 */
	void SkyBoxApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Our scene contains a `nap::CubeMapFromFile` which must be pre-rendered in a headless render pass. This only needs to happen once, and there
		// are no other objects that require headless rendering each frame. Therefore, `isHeadlessCommandQueued` should only be true in the first frame
		// of rendering and record pre-render operations for our cube map resources.
		if (mRenderService->isHeadlessCommandQueued())
		{
			if (mRenderService->beginHeadlessRecording())
				mRenderService->endHeadlessRecording();
		}

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render world
			auto& perspective_camera = mCameraEntity->getComponent<PerspCameraComponentInstance>();
			mRenderService->renderObjects(*mRenderWindow, perspective_camera, render_comps);

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


	/**
	* Occurs when the event handler receives a window message.
	* You generally give it to the render service which in turn forwards it to the right internal window.
	* On the next update the render service automatically processes all window events.
	* If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	*/
	void SkyBoxApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	* Called by the app loop. It's best to forward messages to the input service for further processing later on
	* In this case we also check if we need to toggle full-screen or exit the running app
	*/
	void SkyBoxApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			// Escape the loop when esc is pressed
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit();
			}

			// Toggle fullscreen on 'f'
			else if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}
}
