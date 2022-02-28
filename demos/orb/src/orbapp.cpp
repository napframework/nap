/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "orbapp.h"

 // External Includes
#include <orbcomponent.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <glm/ext.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrbApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool OrbApp::init(utility::ErrorState& errorState)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>();
		mInputService = getCore().getService<InputService>();
		mSceneService = getCore().getService<SceneService>();
		mGuiService = getCore().getService<IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile(getCore().getProjectInfo()->getDataFile(), errorState))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			return false;
		}

		// Gather resources
		if (!reload(errorState))
			return false;

		// Reload the selected preset after hot-reloading 
		mResourceManager->mPreResourcesLoadedSignal.connect(mCacheSlot);
		mResourceManager->mPostResourcesLoadedSignal.connect(mReloadSlot);

		mGuiService->selectWindow(mRenderWindow);

		return true;
	}


	bool OrbApp::reload(utility::ErrorState& errorState)
	{
		rtti::ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mRenderWindow = mResourceManager->findObject<RenderWindow>("RenderWindow");
		mOrthoCameraEntity = scene->findEntity("OrthoCameraEntity");
		mDefaultInputRouter = scene->findEntity("DefaultInputRouterEntity");

		// Get orb entity and component
		mOrbEntity = scene->findEntity("OrbEntity");
		if (!errorState.check(mOrbEntity != nullptr, "Missing OrbEntity"))
			return false;

		if (!errorState.check(mOrbEntity->hasComponent<OrbComponentInstance>(), "Missing 'OrbComponent' in 'OrbEntity'"))
			return false;

		mOrbComponent = &mOrbEntity->getComponent<OrbComponentInstance>();

		// Get world entity - parent of our renderable scene
		mWorldEntity = scene->findEntity("WorldEntity");
		if (!errorState.check(mWorldEntity != nullptr, "Missing WorldEntity"))
			return false;

		mCameraEntity = scene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing CameraEntity"))
			return false;

		// Get render target
		//mRenderTarget = mResourceManager->findObject<RenderTarget>("ColorTarget");
		//if (!errorState.check(mRenderTarget != nullptr, "Missing resource nap::RenderTarget with id 'ColorTarget'"))
		//	return false;

		// Get the camera component
		mPerspCameraComponent = &mCameraEntity->getComponent<PerspCameraComponentInstance>();
		if (!errorState.check(mPerspCameraComponent != nullptr, "Missing component 'nap::PerspCameraComponent'"))
			return false;

		// Load preset
		//if (mSelectedPreset.empty())
		//{
		//	// Load the first preset automatically
		//	auto* parameter_service = getCore().getService<ParameterService>();
		//	auto presets = parameter_service->getPresets(*mParameterGUI->mParameterGroup);
		//	if (!parameter_service->getPresets(*mParameterGUI->mParameterGroup).empty())
		//	{
		//		if (!mParameterGUI->load(presets[0], errorState))
		//			return false;
		//	}
		//}
		//else
		//{
		//	mParameterGUI->load(mSelectedPreset, errorState);
		//}

		// Sample default color values from loaded color palette - overrides preset
		//const auto palette = mGuiService->getPalette();
		//RGBColorFloat diffuse_color = palette.mHighlightColor1.convert<RGBColorFloat>();
		//RGBColorFloat diffuse_color_ex = palette.mHighlightColor2.convert<RGBColorFloat>();
		//RGBColorFloat bg_color = palette.mDarkColor.convert<RGBColorFloat>();

		//mOrbComponent->getResource().mDiffuseColorParam->setValue(diffuse_color);
		//mOrbComponent->getResource().mDiffuseColorExParam->setValue(diffuse_color_ex);

		//mRenderTarget->setClearColor({ mGuiService->getPalette().mDarkColor.convert<RGBColorFloat>(), 1.0f });
		//mRenderWindow->setClearColor({ mGuiService->getPalette().mDarkColor.convert<RGBColorFloat>(), 1.0f });

		return true;
	}


	void OrbApp::cache()
	{
		// Cache preset
		//auto* parameter_service = getCore().getService<ParameterService>();
		//mSelectedPreset = parameter_service->getPresets(*mParameterGUI->mParameterGroup)[mParameterGUI->getSelectedPresetIndex()];
	}


	/**
	 * Forward all received input events to the input router.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our first person camera.
	 *
	 * We also set up our gui that is drawn at a later stage.
	 */
	void OrbApp::update(double deltaTime)
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
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		//mParameterGUI->show(false);
		ImGui::End();

		// Update uniforms
		//mContrastUniform->setValue(mContrastParam->mValue);
		//mBrightnessUniform->setValue(mBrightnessParam->mValue);
		//mSaturationUniform->setValue(mSaturationParam->mValue);
		//mBlendUniform->setValue(mBlendParam->mValue);
	}


	/**
	 * Render all objects to screen at once
	 * In this case that's only the particle mesh
	 */
	void OrbApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording compute commands
		if (mRenderService->beginComputeRecording())
		{
			//mOrbEntity->getComponent<OrbComponentInstance>().compute();
			mRenderService->endComputeRecording();
		}
		
		// Headless
		if (mRenderService->beginHeadlessRecording())
		{
			//std::vector<RenderableComponentInstance*> render_comps;
			//mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

			// Offscreen color pass -> Render all available geometry to ColorTexture
			//mRenderTarget->beginRendering();

			//mRenderService->renderObjects(*mRenderTarget, mCameraEntity->getComponent<PerspCameraComponentInstance>(), render_comps);	
			//mRenderTarget->endRendering();

			// Offscreen contrast pass -> Use ColorTexture as input, ColorTexture_Contrast as output
			//mContrastComponent->draw();

			// Offscreen bloom pass -> Use ColorTexture as input, OutputTexture (internal) as output
			//mBloomComponent->draw();

			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			std::vector<RenderableComponentInstance*> render_comps;
			mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

			// Begin render pass
			mRenderWindow->beginRendering();

			// Render world
			mRenderService->renderObjects(*mRenderWindow, mCameraEntity->getComponent<PerspCameraComponentInstance>(), render_comps);

			// Render composite component
			//mRenderService->renderObjects(*mRenderWindow, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), { mCompositeComponent });

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
	void OrbApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	* Called by the app loop. It's best to forward messages to the input service for further processing later on
	* In this case we also check if we need to toggle full-screen or exit the running app
	*/
	void OrbApp::inputMessageReceived(InputEventPtr inputEvent)
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
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}
}
