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

		// Get the skybox
		mSkyboxEntity = scene->findEntity("SkyBoxEntity");
		if (!errorState.check(mSkyboxEntity != nullptr, "Missing SkyBoxEntity"))
			return false;

		// Get the torus
		mTorusEntity = scene->findEntity("TorusEntity");
		if (!errorState.check(mTorusEntity != nullptr, "Missing TorusEntity"))
			return false;

		// Get camera
		mCameraEntity = scene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing CameraEntity"))
			return false;

		// Cache cube maps
		mCubeMaps = mResourceManager->getObjects<CubeMapFromFile>();

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
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::Text(utility::stringFormat("%.02f fps | %.02f ms", getCore().getFramerate(), deltaTime*1000.0).c_str());

		// Get skybox
		auto& render_skybox = mSkyboxEntity->getComponent<RenderSkyBoxComponentInstance>();

		// Create a list of cubemap labels
		std::vector<const char*> labels;
		labels.reserve(mCubeMaps.size());
		std::for_each(mCubeMaps.begin(), mCubeMaps.end(), [&labels](const auto& cube) {
			labels.emplace_back(cube->mID.c_str());
			});

		// Here we create a simple combobox that allows the user to switch between cube maps
		// On selection, we must update the sampler instance of the sky box and all render components that display its reflection
		if (ImGui::Combo("Cube Maps", &mCubeMapIndex, labels.data(), mCubeMaps.size()))
		{
			// Update the sampler instance of the skybox
			render_skybox.setTexture(*mCubeMaps[mCubeMapIndex]);

			// Update the cube map in the torus -> Normally you would do this in a controlling component.
			// That component would update the cube texture of all compatible materials on initialization and when changed.
			auto& render_torus = mTorusEntity->getComponent<RenderableMeshComponentInstance>();
			auto* sampler = render_torus.getMaterialInstance().getOrCreateSampler<SamplerCubeInstance>(uniform::blinnphongcolor::sampler::environmentMap);
			assert(sampler != nullptr);
			sampler->setTexture(*mCubeMaps[mCubeMapIndex]);
		}

		// Change sky box color
		auto skybox_color = render_skybox.getColor();
		if (ImGui::ColorEdit3("Skybox Color", &skybox_color.x))
		{
			// Set sky box color
			render_skybox.setColor(skybox_color);

			// Sync the torus color -> Normally you would do this in a controlling component using parameters.
			// Where the parameter value is pushed to all compatible materials on initialization and parameter change.
			auto& render_torus = mTorusEntity->getComponent<RenderableMeshComponentInstance>();
			auto* ubo_struct = render_torus.getMaterialInstance().getOrCreateUniform("UBO");
			assert(ubo_struct != nullptr);
			auto* co_uniform = ubo_struct->getOrCreateUniform<UniformVec3Instance>("color");
			assert(co_uniform != nullptr);
			co_uniform->setValue(skybox_color);
		}

		// Change sky box opacity -> reveals clear color of window
		auto skybox_opacity = render_skybox.getOpacity();
		if (ImGui::SliderFloat("Skybox Opacity", &skybox_opacity, 0.0f, 1.0f))
			render_skybox.setOpacity(skybox_opacity);

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

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get components to render
			std::vector<RenderableComponentInstance*> render_comps =
			{
				&mSkyboxEntity->getComponent<RenderableComponentInstance>(),
				&mTorusEntity->getComponent<RenderableComponentInstance>()
			};

			// Render
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
