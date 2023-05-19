/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lightsandshadowapp.h"

 // External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <lightcomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <glm/ext.hpp>
#include <parameternumeric.h>
#include <parameterquat.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightsAndShadowApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool LightsAndShadowApp::init(utility::ErrorState& errorState)
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

		return true;
	}


	/**
	 * Forward all received input events to the input router.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our first person camera.
	 *
	 * We also set up our gui that is drawn at a later stage.
	 */
	void LightsAndShadowApp::update(double deltaTime)
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
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "wasd keys to move, mouse + left mouse button to look");
		ImGui::Text(utility::stringFormat("%.02f fps | %.02f ms", getCore().getFramerate(), deltaTime*1000.0).c_str());

		std::vector<LightComponentInstance*> lights;
		mWorldEntity->getComponentsOfTypeRecursive<LightComponentInstance>(lights);

		if (!lights.empty())
		{
			std::vector<const char*> labels;
			labels.reserve(lights.size());
			std::for_each(lights.begin(), lights.end(), [&labels](const auto& light) {
				labels.emplace_back(light->mID.c_str());
				});

			static int item_index = 0;
			LightComponentInstance* selected_light = lights[item_index];
			if (ImGui::Combo("Selected Light", &item_index, labels.data(), lights.size()))
				selected_light = lights[item_index];

			// Calculate position -> default (from initialization) + offset 
			auto xform_it = mLightXform.try_emplace(lights[item_index]->mID, glm::vec3(0.0f, 0.0f, 0.0f));
			if (ImGui::SliderFloat3("Translate", glm::value_ptr(xform_it.first->second), -10.0f, 10.0f))
			{
				auto* default_xform = rtti_cast<nap::TransformComponent>(selected_light->getTransform().getComponent());
				selected_light->getTransform().setTranslate(default_xform->mProperties.mTranslate + xform_it.first->second);
			}

			// Calculate rotation -> default (from initialization) + offset
			auto euler_it = mLightEuler.try_emplace(lights[item_index]->mID, glm::vec3(0.0f, 0.0f, 0.0f));
			if (ImGui::SliderFloat3("Rotate", glm::value_ptr(euler_it.first->second), -180.0f, 180.0f))
			{
				auto* default_xform = rtti_cast<nap::TransformComponent>(selected_light->getTransform().getComponent());
				glm::vec3 rot_euler = math::radians(default_xform->mProperties.mRotate + euler_it.first->second);
				selected_light->getTransform().setRotate(rot_euler);
			}

			bool enabled = selected_light->isEnabled();
			if (ImGui::Checkbox("Enable", &enabled))
				selected_light->enable(enabled);
		}

		mResourceManager->findObject<ParameterGUI>("ParameterGUI")->show(false);

		ImGui::End();
	}


	/**
	 * Render all objects to screen at once
	 * In this case that's only the particle mesh
	 */
	void LightsAndShadowApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Shadow pass
		if (mRenderService->beginHeadlessRecording())
		{
			mRenderAdvancedService->renderShadows(render_comps, true, mRenderService->findRenderMask("Shadow"));
			mRenderService->endHeadlessRecording();
		}

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
	void LightsAndShadowApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	* Called by the app loop. It's best to forward messages to the input service for further processing later on
	* In this case we also check if we need to toggle full-screen or exit the running app
	*/
	void LightsAndShadowApp::inputMessageReceived(InputEventPtr inputEvent)
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
