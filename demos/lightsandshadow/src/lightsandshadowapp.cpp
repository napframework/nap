/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lightsandshadowapp.h"

 // External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <lightcomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <glm/ext.hpp>
#include <parameternumeric.h>
#include <spotlightcomponent.h>
#include <directionallightcomponent.h>
#include <pointlightcomponent.h>
#include <rotatecomponent.h>

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

		mPointLightEntity = scene->findEntity("PointLightEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing PointLightEntity"))
			return false;

		mSpotLightEntity = scene->findEntity("SpotLightEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing SpotLightEntity"))
			return false;

		mSunLightEntity = scene->findEntity("SunLightEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing SunLightEntity"))
			return false;

		mShadowTag = mResourceManager->findObject("RenderTag_Shadow");
		if (!errorState.check(mShadowTag != nullptr, "Missing shadow render tag"))
			return false;

		return true;
	}


	/**
	 * Utility function that pushes shared light controls
	 */
	static void pushDefaultLightControls(LightComponentInstance& light)
	{
		bool enabled = light.isEnabled();
		if (ImGui::Checkbox("Enable", &enabled))
			light.enable(enabled);

		auto color = light.getColor();
		if (ImGui::ColorEdit3("Color", color.getData()))
			light.setColor(color);

		auto inten = light.getIntensity();
		if (ImGui::SliderFloat("Intensity", &inten, 0.0f, 5.0f, "%.3f", 2.0f))
			light.setIntensity(inten);

		auto shadow = light.getShadowStrength();
		if (ImGui::SliderFloat("Shadow Strength", &shadow, 0.0f, 1.0f, "%.3f", 1.0f))
			light.setShadowStrength(shadow);
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

		ImGui::Checkbox("Show Light Origin", &mShowLocators);
		if (mShowLocators)
		{
			ImGui::SameLine();
			ImGui::Checkbox("Show Shadow Frustrum", &mShowFrustrum);
		}

		if (ImGui::CollapsingHeader("Spotlight"))
		{
			auto& spot_comp = mSpotLightEntity->getComponent<SpotLightComponentInstance>();
			ImGui::PushID(&spot_comp);
			pushDefaultLightControls(spot_comp);

			auto attenuation = spot_comp.getAttenuation();
			if (ImGui::SliderFloat("Attenuation", &attenuation, 0.0f, 1.0f))
				spot_comp.setAttenuation(attenuation);

			auto angle = spot_comp.getAngle();
			if (ImGui::SliderFloat("Angle", &angle, 1.0f, 180.0f))
				spot_comp.setAngle(angle);

			auto falloff = spot_comp.getFalloff();
			if (ImGui::SliderFloat("Falloff", &falloff, 0.0f, 1.0f))
				spot_comp.setFalloff(falloff);

			auto fov = spot_comp.getFieldOfView();
			if (ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f))
				spot_comp.setFieldOfView(fov);

			ImGui::PopID();
		}
		if (ImGui::CollapsingHeader("Sunlight"))
		{
			auto& sun_comp = mSunLightEntity->getComponent<DirectionalLightComponentInstance>();
			ImGui::PushID(&sun_comp);
			pushDefaultLightControls(sun_comp);
			ImGui::PopID();
		}
		if(ImGui::CollapsingHeader("Pointlight"))
		{
			auto& point_comp = mPointLightEntity->getComponent<PointLightComponentInstance>();
			ImGui::PushID(&point_comp);
			pushDefaultLightControls(point_comp);

			auto attenuation = point_comp.getAttenuation();
			if (ImGui::SliderFloat("Attenuation", &attenuation, 0.0f, 1.0f))
				point_comp.setAttenuation(attenuation);

			auto& rotate_comp = mPointLightEntity->getParent()->getComponent<RotateComponentInstance>();
			float rotate = rotate_comp.getSpeed();
			if (ImGui::SliderFloat("Rotation Speed", &rotate, -0.5f, 0.5f))
				rotate_comp.setSpeed(rotate);

			ImGui::PopID();
		}
		ImGui::End();
	}


	/**
	 * Render all objects to screen
	 */
	void LightsAndShadowApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Record shadow map rendering operations to the headless command buffer. The `renderShadows` call in the RenderAdvanced service does this for all
		// lights that have shadows enabled. When the `updateMaterials` argument is set to true, light uniform and sampler data is also updated.
		if (mRenderService->beginHeadlessRecording())
		{
			mRenderAdvancedService->renderShadows(render_comps, true, *mShadowTag);
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

			// Render light locators
			if (mShowLocators)
				mRenderAdvancedService->renderLocators(*mRenderWindow, perspective_camera, mShowFrustrum);

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
