/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "computeflockingapp.h"

 // External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <orthocameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <flockingsystemcomponent.h>
#include <renderbloomcomponent.h>
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

		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window0");
		mCameraEntity = scene->findEntity("CameraEntity");
		mOrthoCameraEntity = scene->findEntity("OrthoCameraEntity");
		mDefaultInputRouter = scene->findEntity("DefaultInputRouterEntity");
		mFlockingSystemEntity = scene->findEntity("FlockingSystemEntity");
		mForegroundEntity = scene->findEntity("ForegroundEntity");

		if (!errorState.check(mFlockingSystemEntity != nullptr, "Missing FlockingSystemEntity"))
			return false;

		// Create render texture
		mRenderTexture = mResourceManager->findObject<RenderTexture2D>("ColorRT");
		if (mRenderTexture == nullptr)
		{
			errorState.fail("%s: Missing nap::RenderTexture2D 'ColorRT'", mRenderTexture->mID.c_str());
			return false;
		}

		// Create render target
		mRenderTarget = mResourceManager->createObject<RenderTarget>();
		mRenderTarget->mColorTexture = mRenderTexture;
		mRenderTarget->mRequestedSamples = mRenderWindow->mRequestedSamples;
		mRenderTarget->mClearColor = mRenderWindow->mClearColor;
		mRenderTarget->mSampleShading = mRenderWindow->mSampleShading;
		if (!mRenderTarget->init(errorState))
		{
			errorState.fail("%s: Failed to initialize internal render target", mRenderTarget->mID.c_str());
			return false;
		}

		// Get foreground component responsible for rendering a screen texture
		RenderableMeshComponentInstance& foreground_comp = mForegroundEntity->getComponent<RenderableMeshComponentInstance>();

		// Get the bloom component responsible for producing a bloom texture
		RenderBloomComponentInstance& bloom_comp = mForegroundEntity->getComponent<RenderBloomComponentInstance>();

		// Get the sampler instance for compositing bloom and color
		Sampler2DArrayInstance* sampler_instance = static_cast<Sampler2DArrayInstance*>(foreground_comp.getMaterialInstance().findSampler("colorTextures"));

		// Set the output texture of the render blzoom component to the last index of the 
		sampler_instance->setTexture(1, bloom_comp.getOutputTexture());

		auto* ubo_struct = foreground_comp.getMaterialInstance().findUniform("UBO");
		mBloomValueUniform = ubo_struct->findUniform<UniformFloatInstance>("mixValue");
		mBloomValue = mBloomValueUniform->getValue();

		mNumBoids = mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>().mNumBoids;

		mParameterGUI = std::make_unique<ParameterGUI>(getCore());
		mParameterGUI->mParameterGroup = mResourceManager->findObject<ParameterGroup>("FlockingParameters");

		if (!errorState.check(mParameterGUI->mParameterGroup != nullptr, "Missing ParameterGroup 'FlockingParameters'"))
			return false;

		// Reload the selected preset after hot-reloading 
		mResourceManager->mPreResourcesLoadedSignal.connect(mCacheSelectedPresetSlot);
		mResourceManager->mPostResourcesLoadedSignal.connect(mReloadSelectedPresetSlot);

		// Load the first preset automatically
		auto* parameter_service = getCore().getService<ParameterService>();
		auto presets = parameter_service->getPresets(*mParameterGUI->mParameterGroup);
		if (!parameter_service->getPresets(*mParameterGUI->mParameterGroup).empty())
		{
			if (!mParameterGUI->load(presets[0], errorState))
				return false;
		}

		mGuiService->selectWindow(mRenderWindow);

		return true;
	}


	void ComputeFlockingApp::reloadSelectedPreset()
	{
		// Load the first preset automatically
		auto* parameter_service = getCore().getService<ParameterService>();
		utility::ErrorState error_state;
		mParameterGUI->load(mSelectedPreset, error_state);
	}


	void ComputeFlockingApp::cacheSelectedPreset()
	{
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

		// Update GUI
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "wasd keys to move, mouse + left mouse button to look");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::Text(utility::stringFormat("Boids: %d", mNumBoids).c_str());
		ImGui::SliderFloat("Bloom", &mBloomValue, 0.0f, 1.0f);
		mParameterGUI->show(false);
		ImGui::End();

		// Update uniforms
		mBloomValueUniform->setValue(mBloomValue);
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

		// Exclude components from rendering
		RenderableMeshComponentInstance& foreground_comp = mForegroundEntity->getComponent<RenderableMeshComponentInstance>();
		foreground_comp.setVisible(false);

		RenderBloomComponentInstance& bloom_comp = mForegroundEntity->getComponent<RenderBloomComponentInstance>();
		bloom_comp.setVisible(false);
		
		// Headless
		if (mRenderService->beginHeadlessRecording())
		{
			// Offscreen color pass -> Render all available geometry to ColorRT
			mRenderTarget->beginRendering();
			mRenderService->renderObjects(*mRenderTarget, mCameraEntity->getComponent<PerspCameraComponentInstance>());	
			mRenderTarget->endRendering();

			// Offscreen bloom pass -> Use ColorRT as input texture
			bloom_comp.setVisible(true);
			bloom_comp.draw();
			bloom_comp.setVisible(false);

			mRenderService->endHeadlessRecording();
		}

		// Include again
		foreground_comp.setVisible(true);

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Render plane -> foreground_comp has ColorRT as sampler
			mRenderService->renderObjects(*mRenderWindow, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), { &foreground_comp });

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
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}
}
