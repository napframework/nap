/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "computeflockingapp.h"
#include "flockingsystemcomponent.h"
#include "boidtargettranslatecomponent.h"

 // External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <flockingsystemcomponent.h>
#include <glm/ext.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeFlockingApp)
RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool ComputeFlockingApp::init(utility::ErrorState& errorState)
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

		return true;
	}


	bool ComputeFlockingApp::reload(utility::ErrorState& errorState)
	{
		rtti::ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mRenderWindow = mResourceManager->findObject<RenderWindow>("RenderWindow");
		mOrthoCameraEntity = scene->findEntity("OrthoCameraEntity");
		mDefaultInputRouter = scene->findEntity("DefaultInputRouterEntity");

		// Get flocking system entity and component
		mFlockingSystemEntity = scene->findEntity("FlockingSystemEntity");
		if (!errorState.check(mFlockingSystemEntity != nullptr, "Missing FlockingSystemEntity"))
			return false;

		if (!errorState.check(mFlockingSystemEntity->hasComponent<FlockingSystemComponentInstance>(), "Missing 'FlockingSystemComponent' in 'FlockingSystemEntity'"))
			return false;

		mFlockingSystemComponent = &mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>();

		// Get world entity - parent of our renderable scene
		mWorldEntity = scene->findEntity("WorldEntity");
		if (!errorState.check(mWorldEntity != nullptr, "Missing WorldEntity"))
			return false;

		mCameraEntity = scene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing CameraEntity"))
			return false;

		mRenderEntity = scene->findEntity("RenderEntity");
		if (!errorState.check(mRenderEntity != nullptr, "Missing RenderEntity"))
			return false;

		mBoundsEntity = scene->findEntity("BoundsEntity");
		if (!errorState.check(mBoundsEntity != nullptr, "Missing BoundsEntity"))
			return false;

		// Get render target
		mRenderTarget = mResourceManager->findObject<RenderTarget>("ColorTarget");
		if (!errorState.check(mRenderTarget != nullptr, "Missing resource nap::RenderTarget with id 'ColorTarget'"))
			return false;

		mContrastComponent = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("RenderContrast");
		if (!errorState.check(mContrastComponent != nullptr, "Missing component nap::RenderToTextureComponentInstance with id 'RenderContrast'"))
			return false;

		// Get composite component responsible for rendering final texture
		mCompositeComponent = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("RenderComposite");
		if (!errorState.check(mCompositeComponent != nullptr, "Missing component nap::RenderToTextureComponentInstance with id 'RenderComposite'"))
			return false;

		// Get the bloom component responsible for producing a bloom texture
		mBloomComponent = &mRenderEntity->getComponent<RenderBloomComponentInstance>();
		if (!errorState.check(mBloomComponent != nullptr, "Missing component nap::RenderBloomComponent with id 'RenderBloom'"))
			return false;

		// Get the bounds mesh component
		mBoundsAtmosphereMeshComponent = mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsFill");
		if (!errorState.check(mBoundsAtmosphereMeshComponent != nullptr, "'BoundsEntity' is missing component 'nap::RenderableMeshcomponent' with id 'RenderBoundsFill'"))
			return false;

		// Get the bounds wire frame component
		mBoundsWireMeshComponent = mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsWireFrame");
		if (!errorState.check(mBoundsWireMeshComponent != nullptr, "'BoundsEntity' is missing component 'nap::RenderableMeshcomponent' with id 'RenderBoundsWireFrame'"))
			return false;

		// Get the camera component
		mPerspCameraComponent = &mCameraEntity->getComponent<PerspCameraComponentInstance>();
		if (!errorState.check(mPerspCameraComponent != nullptr, "Missing component 'nap::PerspCameraComponent'"))
			return false;

		// Get boid target point mesh and translate component
		const auto boid_target_entity = scene->findEntity("BoidTargetEntity");
		if (boid_target_entity != nullptr)
		{
			mTargetPointMeshComponent = &boid_target_entity->getComponent<RenderableMeshComponentInstance>();
			mBoidTargetTranslateComponent = &boid_target_entity->getComponent<BoidTargetTranslateComponentInstance>();
		}

		// Get the sampler instance for compositing bloom and color
		Sampler2DArrayInstance* sampler_instance = static_cast<Sampler2DArrayInstance*>(mCompositeComponent->getMaterialInstance().findSampler("colorTextures"));

		// Set the output texture of the render bloom component
		sampler_instance->setTexture(1, mBloomComponent->getOutputTexture());

		// Find uniforms
		UniformStructInstance* ubo_struct = mContrastComponent->getMaterialInstance().findUniform("UBO");
		mContrastUniform = ubo_struct->findUniform<UniformFloatInstance>("contrast");
		mBrightnessUniform = ubo_struct->findUniform<UniformFloatInstance>("brightness");
		mSaturationUniform = ubo_struct->findUniform<UniformFloatInstance>("saturation");

		ubo_struct = mCompositeComponent->getMaterialInstance().findUniform("UBO");
		mBlendUniform = ubo_struct->findUniform<UniformFloatInstance>("blend");

		ubo_struct = mBoundsAtmosphereMeshComponent->getMaterialInstance().findUniform("Vert_UBO");
		mBoundsCameraPositionUniform = ubo_struct->findUniform<UniformVec3Instance>("cameraLocation");

		// Cache boid count
		mNumBoids = mFlockingSystemComponent->mNumBoids;

		mParameterGUI = std::make_unique<ParameterGUI>(getCore());
		mParameterGUI->mParameterGroup = mResourceManager->findObject<ParameterGroup>("FlockingParameters");

		if (!errorState.check(mParameterGUI->mParameterGroup != nullptr, "Missing ParameterGroup 'FlockingParameters'"))
			return false;

		mContrastParam = rtti_cast<ParameterFloat>(mParameterGUI->mParameterGroup->findParameterRecursive("ContrastParameter").get());
		if (!errorState.check(mContrastParam != nullptr, "Missing float parameter 'ContrastParameter'"))
			return false;

		mBrightnessParam = rtti_cast<ParameterFloat>(mParameterGUI->mParameterGroup->findParameterRecursive("BrightnessParameter").get());
		if (!errorState.check(mBrightnessParam != nullptr, "Missing float parameter 'BrightnessParameter'"))
			return false;

		mSaturationParam = rtti_cast<ParameterFloat>(mParameterGUI->mParameterGroup->findParameterRecursive("SaturationParameter").get());
		if (!errorState.check(mSaturationParam != nullptr, "Missing float parameter 'SaturationParameter'"))
			return false;

		mBlendParam = rtti_cast<ParameterFloat>(mParameterGUI->mParameterGroup->findParameterRecursive("BloomBlendParameter").get());
		if (!errorState.check(mBlendParam != nullptr, "Missing float parameter 'BloomBlendParameter'"))
			return false;

		mShowBoundsParam = rtti_cast<ParameterBool>(mParameterGUI->mParameterGroup->findParameterRecursive("ShowBoundsParameter").get());
		if (!errorState.check(mShowBoundsParam != nullptr, "Missing bool parameter 'ShowBoundsParameter'"))
			return false;

		mBoundsRadiusParam = mFlockingSystemComponent->getResource().mBoundsRadiusParam.get();

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

		// Sample default color values from loaded color palette - overrides preset
		const auto palette = mGuiService->getPalette();
		mFlockingSystemComponent->getResource().mDiffuseColorParam->setValue(palette.mHighlightColor1);
		mFlockingSystemComponent->getResource().mDiffuseColorExParam->setValue(palette.mHighlightColor2);
		mFlockingSystemComponent->getResource().mLightColorParam->setValue(palette.mHighlightColor4);
		mFlockingSystemComponent->getResource().mHaloColorParam->setValue(palette.mHighlightColor4);
		mFlockingSystemComponent->getResource().mSpecularColorParam->setValue(palette.mHighlightColor2);

		mRenderTarget->setClearColor({ mGuiService->getPalette().mDarkColor.convert<RGBColorFloat>(), 1.0f });
		mRenderWindow->setClearColor({ mGuiService->getPalette().mDarkColor.convert<RGBColorFloat>(), 1.0f });

		// Set boid target color
		if (mTargetPointMeshComponent != nullptr)
		{
			auto* ubo_struct = mTargetPointMeshComponent->getMaterialInstance().getOrCreateUniform("UBO");
			ubo_struct->getOrCreateUniform<UniformVec3Instance>("color")->setValue(palette.mHighlightColor4.convert<RGBColorFloat>().toVec3());
		}

		return true;
	}


	void ComputeFlockingApp::cache()
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
	void ComputeFlockingApp::update(double deltaTime)
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

		// Prepare gui if requested
		if (mShowGUI)
		{
			ImGui::Begin("Controls");
			ImGui::Text(getCurrentDateTime().toString().c_str());
			ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "'wasd' keys to move, mouse + left mouse button to look");
			ImGui::TextColored(mGuiService->getPalette().mHighlightColor3, "press 'h' to hide this window");
			ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::Text(utility::stringFormat("Boids: %d", mNumBoids).c_str());
			mParameterGUI->show(false);
			ImGui::End();
		}

		// Update uniforms
		mContrastUniform->setValue(mContrastParam->mValue);
		mBrightnessUniform->setValue(mBrightnessParam->mValue);
		mSaturationUniform->setValue(mSaturationParam->mValue);
		mBlendUniform->setValue(mBlendParam->mValue);
		
		// Update bound scale
		auto& transform = mBoundsEntity->getComponent<TransformComponentInstance>();
		transform.setScale({ mBoundsRadiusParam->mValue, mBoundsRadiusParam->mValue, mBoundsRadiusParam->mValue });

		// Update target parent transform scale
		if (mBoidTargetTranslateComponent != nullptr)
			mBoidTargetTranslateComponent->mRadius = mBoundsRadiusParam->mValue;

		// Update camera position
		auto& camera_transform = mPerspCameraComponent->getEntityInstance()->getComponent<TransformComponentInstance>();
		mBoundsCameraPositionUniform->setValue(camera_transform.getTranslate());

		// Update if we need to display bounds
		mBoundsWireMeshComponent->setVisible(mShowBoundsParam->getValue());
		mBoundsAtmosphereMeshComponent->setVisible(mShowBoundsParam->getValue());
	}


	/**
	 * Render all objects to screen at once
	 * In this case that's only the particle mesh
	 */
	void ComputeFlockingApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording compute commands
		if (mRenderService->beginComputeRecording())
		{
			mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>().compute();
			mRenderService->endComputeRecording();
		}
		
		// Headless
		if (mRenderService->beginHeadlessRecording())
		{
			std::vector<RenderableComponentInstance*> render_comps;
			mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

			// Offscreen color pass -> Render all available geometry to ColorTexture
			mRenderTarget->beginRendering();
			mRenderService->renderObjects(*mRenderTarget, mCameraEntity->getComponent<PerspCameraComponentInstance>(), render_comps);	
			mRenderTarget->endRendering();

			// Offscreen contrast pass -> Use ColorTexture as input, ColorTexture_Contrast as output
			mContrastComponent->draw();

			// Offscreen bloom pass -> Use ColorTexture as input, OutputTexture (internal) as output
			mBloomComponent->draw();

			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render composite component
			mRenderService->renderObjects(*mRenderWindow, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), { mCompositeComponent });

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
	void ComputeFlockingApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	* Called by the app loop. It's best to forward messages to the input service for further processing later on
	* In this case we also check if we need to toggle full-screen or exit the running app
	*/
	void ComputeFlockingApp::inputMessageReceived(InputEventPtr inputEvent)
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
			// Toggle show gui
			else if (press_event->mKey == nap::EKeyCode::KEY_h)
			{
				mShowGUI = !mShowGUI;
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}
}
