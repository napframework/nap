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
#include <lightcomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <glm/ext.hpp>
#include <parameternumeric.h>

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
		mRenderService			= getCore().getService<RenderService>();
		mRenderAdvancedService	= getCore().getService<RenderAdvancedService>();
		mInputService			= getCore().getService<InputService>();
		mSceneService			= getCore().getService<SceneService>();
		mGuiService				= getCore().getService<IMGuiService>();

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

		// Get the camera component
		mPerspCameraComponent = &mCameraEntity->getComponent<PerspCameraComponentInstance>();
		if (!errorState.check(mPerspCameraComponent != nullptr, "Missing component 'nap::PerspCameraComponent'"))
			return false;

		mParameterGUI = std::make_unique<ParameterGUI>(getCore());
		mParameterGUI->mParameterGroup = mResourceManager->findObject<ParameterGroup>("OrbParameters");

		if (!errorState.check(mParameterGUI->mParameterGroup != nullptr, "Missing ParameterGroup 'FlockingParameters'"))
			return false;

		// Load preset
		if (mSelectedPreset.empty())
		{
			// Load the first preset automatically
			auto* parameter_service = getCore().getService<ParameterService>();
			auto presets = parameter_service->getPresets(*mParameterGUI->mParameterGroup);
			if (!parameter_service->getPresets(*mParameterGUI->mParameterGroup).empty())
			{
				if (!mParameterGUI->load(presets[0], errorState))
					return false;
			}
		}
		else
		{
			mParameterGUI->load(mSelectedPreset, errorState);
		}

		// Get highlight color from palette
		mRenderWindow->setClearColor({ mGuiService->getPalette().mDarkColor.convert<RGBColorFloat>(), 1.0f });

		// Cache light components
		mLightComponents.clear();
		mWorldEntity->getComponentsOfTypeRecursive<LightComponentInstance>(mLightComponents);

		return true;
	}


	void OrbApp::cache()
	{
		// Cache preset
		auto* parameter_service = getCore().getService<ParameterService>();
		mSelectedPreset = parameter_service->getPresets(*mParameterGUI->mParameterGroup)[mParameterGUI->getSelectedPresetIndex()];
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
		ImGui::Text(utility::stringFormat("%.02f fps | %.02f ms", getCore().getFramerate(), deltaTime*1000.0).c_str());

		//for (auto& light : mLightComponents)
		//{
		//	if (!light->isShadowEnabled())
		//		continue;

		//	ImGui::Image(light->getShadowTarget().getDepthTexture(), { 200.0f, 200.0f });
		//}

		mParameterGUI->show(false);
		ImGui::End();
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

		std::vector<RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

		// Update bias uniforms
		//for (auto* comp : render_comps)
		//{
		//	if (comp->get_type().is_derived_from(RTTI_OF(RenderableMeshComponentInstance)))
		//	{
		//		auto* mesh = static_cast<RenderableMeshComponentInstance*>(comp);
		//		auto* uni = mesh->getMaterialInstance().findUniform("UBO");
		//		if (uni != nullptr)
		//		{
		//			auto* bias = uni->getOrCreateUniform<UniformFloatInstance>("bias");
		//			if (bias != nullptr)
		//				bias->setValue(mResourceManager->findObject<ParameterFloat>("BiasParameter")->mValue);
		//		}
		//	}
		//}
	
		// Shadow pass
		if (mRenderService->beginHeadlessRecording())
		{
			mRenderAdvancedService->renderShadows(render_comps);
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
