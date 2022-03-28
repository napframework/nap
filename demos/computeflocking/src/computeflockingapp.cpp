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
#include <imguiutils.h>

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

		const auto scene		= mResourceManager->findObject<Scene>("Scene");
		mRenderWindow			= mResourceManager->findObject<RenderWindow>("RenderWindow");
		mOrthoCameraEntity		= scene->findEntity("OrthoCameraEntity");
		mDefaultInputRouter		= scene->findEntity("DefaultInputRouterEntity");

		// Get flocking system entity and ensure component
		mFlockingSystemEntity = scene->findEntity("FlockingSystemEntity");
		if (!errorState.check(mFlockingSystemEntity != nullptr, "Missing entity 'FlockingSystemEntity'"))
			return false;

		// Ensure the flocking system component exists
		if (!errorState.check(mFlockingSystemEntity->hasComponent<FlockingSystemComponentInstance>(), "Missing 'FlockingSystemComponent' in 'FlockingSystemEntity'"))
			return false;

		// Get world entity - parent of our renderable scene
		mWorldEntity = scene->findEntity("WorldEntity");
		if (!errorState.check(mWorldEntity != nullptr, "Missing entity 'WorldEntity'"))
			return false;

		mCameraEntity = scene->findEntity("CameraEntity");
		if (!errorState.check(mCameraEntity != nullptr, "Missing entity 'CameraEntity'"))
			return false;

		mRenderEntity = scene->findEntity("RenderEntity");
		if (!errorState.check(mRenderEntity != nullptr, "Missing entity 'RenderEntity'"))
			return false;

		mBoundsEntity = scene->findEntity("BoundsEntity");
		if (!errorState.check(mBoundsEntity != nullptr, "Missing entity 'BoundsEntity'"))
			return false;

		mTargetEntity = scene->findEntity("BoidTargetEntity");
		if (!errorState.check(mBoundsEntity != nullptr, "Missing entity 'BoidTargetEntity'"))
			return false;

		// Get render target
		mRenderTarget = mResourceManager->findObject<RenderTarget>("ColorTarget");
		if (!errorState.check(mRenderTarget != nullptr, "Missing resource nap::RenderTarget with id 'ColorTarget'"))
			return false;

		// Get the bounds mesh component
		auto* bounds_atmosphere = mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsFill");
		if (!errorState.check(bounds_atmosphere != nullptr, "'BoundsEntity' is missing component 'nap::RenderableMeshcomponent' with id 'RenderBoundsFill'"))
			return false;

		// Get the bounds wire frame component
		auto* bounds_wireframe = mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsWireFrame");
		if (!errorState.check(bounds_wireframe != nullptr, "'BoundsEntity' is missing component 'nap::RenderableMeshcomponent' with id 'RenderBoundsWireFrame'"))
			return false;

		// Get composite component responsible for rendering final texture
		auto* composite_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("BlendTogether");
		if (!errorState.check(composite_comp != nullptr, "Missing component nap::RenderToTextureComponentInstance with id 'RenderComposite'"))
			return false;

		// Find uniforms
		auto* contrast_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("ChangeColor");
		if (!errorState.check(contrast_comp != nullptr, "Missing component nap::RenderToTextureComponentInstance with id 'RenderContrast'"))
			return false;

		// Cache boid count
		auto& flocking_system = mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>();

		// Create parameter GUI
		mParameterGUI = std::make_unique<ParameterGUI>(getCore());

		auto parameter_group = mResourceManager->findObject<ParameterGroup>("FlockingParameters");
		if (!errorState.check(parameter_group != nullptr, "Missing ParameterGroup 'FlockingParameters'"))
			return false;

		mContrastParam = parameter_group->findParameterRecursive("ContrastParameter");
		if (!errorState.check(mContrastParam != nullptr, "Missing float parameter 'ContrastParameter'"))
			return false;

		mBrightnessParam = parameter_group->findParameterRecursive("BrightnessParameter");
		if (!errorState.check(mBrightnessParam != nullptr, "Missing float parameter 'BrightnessParameter'"))
			return false;

		mSaturationParam = parameter_group->findParameterRecursive("SaturationParameter");
		if (!errorState.check(mSaturationParam != nullptr, "Missing float parameter 'SaturationParameter'"))
			return false;

		mBlendParam = parameter_group->findParameterRecursive("BloomBlendParameter");
		if (!errorState.check(mBlendParam != nullptr, "Missing float parameter 'BloomBlendParameter'"))
			return false;

		mShowBoundsParam = parameter_group->findParameterRecursive("ShowBoundsParameter");
		if (!errorState.check(mShowBoundsParam != nullptr, "Missing bool parameter 'ShowBoundsParameter'"))
			return false;

		mBoundsRadiusParam = flocking_system.getResource().mBoundsRadiusParam;

		// Set the parameter group
		mParameterGUI->mParameterGroup = parameter_group;

		// Sample default color values from loaded color palette - overrides preset
		const auto& palette = mGuiService->getPalette();
		flocking_system.getResource().mDiffuseColorParam->setValue(palette.mHighlightColor1);
		flocking_system.getResource().mDiffuseColorExParam->setValue(palette.mHighlightColor2);
		flocking_system.getResource().mLightColorParam->setValue(palette.mHighlightColor4);
		flocking_system.getResource().mHaloColorParam->setValue(palette.mHighlightColor4);
		flocking_system.getResource().mSpecularColorParam->setValue(palette.mHighlightColor2);

		mRenderTarget->setClearColor({ palette.mDarkColor.convert<RGBColorFloat>(), 1.0f });
		mRenderWindow->setClearColor({ palette.mDarkColor.convert<RGBColorFloat>(), 1.0f });

		// Set boid target color
		auto* target_point = &mTargetEntity->getComponent<RenderableMeshComponentInstance>();
		if (target_point != nullptr)
		{
			auto* ubo_struct = target_point->getMaterialInstance().getOrCreateUniform("UBO");
			ubo_struct->getOrCreateUniform<UniformVec3Instance>("color")->setValue(palette.mHighlightColor4.convert<RGBColorFloat>().toVec3());
		}

		return true;
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
		DefaultInputRouter& input_router			= mDefaultInputRouter->getComponent<DefaultInputRouterComponentInstance>().mInputRouter;
		{
			// Update input for first window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntity.get());

			Window* window = mRenderWindow.get();
			mInputService->processWindowEvents(*window, input_router, entities);
		}

		// Fetch render components
		auto* coloradjust_comp						= mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("ChangeColor");
		auto* bloom_comp							= mRenderEntity->findComponentByID<RenderBloomComponentInstance>("ApplyBloom");
		auto* composite_comp						= mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("BlendTogether");

		// Fetch flocking system
		auto& flock_comp							= mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>();

		// Prepare gui
		if (mShowGUI)
		{
			ImGui::Begin("Controls");
			ImGui::Text(getCurrentDateTime().toString().c_str());
			ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "'wasd' keys to move, mouse + left mouse button to look");
			ImGui::TextColored(mGuiService->getPalette().mHighlightColor3, "press 'h' to hide this window");
			ImGui::Text(utility::stringFormat("Rendering Resolution: %d x %d", composite_comp->getOutputTexture().getWidth(), composite_comp->getOutputTexture().getHeight()).c_str());
			ImGui::Text(utility::stringFormat("Performance: %.02f fps | %.02f ms", getCore().getFramerate(), deltaTime*1000.0).c_str());
			ImGui::Text(utility::stringFormat("Boids: %d", flock_comp.getNumBoids()).c_str());
			mParameterGUI->show(false);

			ImGui::Separator();
			if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const float aspect = coloradjust_comp->getOutputTexture().getHeight() / static_cast<float>(coloradjust_comp->getOutputTexture().getWidth());
				const float width = ImGui::GetWindowContentRegionWidth();
				ImGui::Image(coloradjust_comp->getOutputTexture(), { width, width * aspect });
			}
			ImGui::End();
		}

		// Update color adjustment material
		UniformStructInstance* ubo_struct			= coloradjust_comp->getMaterialInstance().getOrCreateUniform("UBO");
		UniformFloatInstance* contrast_uni			= ubo_struct->getOrCreateUniform<UniformFloatInstance>("contrast");
		UniformFloatInstance* brightness_uni		= ubo_struct->getOrCreateUniform<UniformFloatInstance>("brightness");
		UniformFloatInstance* saturation_uni		= ubo_struct->getOrCreateUniform<UniformFloatInstance>("saturation");

		contrast_uni->setValue(rtti_cast<ParameterFloat>(mContrastParam.get())->mValue);
		brightness_uni->setValue(rtti_cast<ParameterFloat>(mBrightnessParam.get())->mValue);
		saturation_uni->setValue(rtti_cast<ParameterFloat>(mSaturationParam.get())->mValue);

		// Update composite material
		ubo_struct									= composite_comp->getMaterialInstance().getOrCreateUniform("FRAGUBO");
		UniformFloatInstance* blend_uni				= ubo_struct->getOrCreateUniform<UniformFloatInstance>("blend");

		blend_uni->setValue(rtti_cast<ParameterFloat>(mBlendParam.get())->mValue);
		
		// Update bound scale
		const float radius							= mBoundsRadiusParam->mValue;
		auto& transform								= mBoundsEntity->getComponent<TransformComponentInstance>();

		transform.setScale({ radius, radius, radius });

		// Update target parent transform scale
		auto& translate_comp						= mTargetEntity->getComponent<BoidTargetTranslateComponentInstance>();
		translate_comp.mRadius						= radius;

		// Update camera location
		auto* atmosphere_comp						= mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsFill");
		ubo_struct									= atmosphere_comp->getMaterialInstance().getOrCreateUniform("VERTUBO");
		UniformVec3Instance* cam_location_uni		= ubo_struct->getOrCreateUniform<UniformVec3Instance>("cameraLocation");

		cam_location_uni->setValue(mCameraEntity->getComponent<TransformComponentInstance>().getTranslate());

		// Update if we need to display bounds and gnomon
		bool show = rtti_cast<ParameterBool>(mShowBoundsParam.get())->mValue;
		mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsWireFrame")->setVisible(show);
		mBoundsEntity->findComponentByID<RenderableMeshComponentInstance>("RenderBoundsFill")->setVisible(show);
		mWorldEntity->getComponent<RenderGnomonComponentInstance>().setVisible(show);
	}


	/**
	 * Render objects offscreen, apply a stack of post-processing effects to it, then render the reesult to a window.
	 * Compute work is also dispatched here as this must happen inside a frame when compute command buffer recording is activated.
	 */
	void ComputeFlockingApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording compute commands.
		// Compute commands are pushed to a separate compute command buffer. This must also always happen before the recording of any rendering commands.
		// within a single frame (including headless). This order is enforced to ensure correct synchronization and simplicity.
		if (mRenderService->beginComputeRecording())
		{
			// Fetch the flocking system component instance from the particle entity
			auto& flock_system = mFlockingSystemEntity->getComponent<FlockingSystemComponentInstance>();

			// The component instance exposes a compute function which selects the correct compute instance to use in the current frame,
			// updates its compute material and dispatches the compute shader. Resources that are accessed in both compute and
			// graphics within the same frame are guarded by memory and execution barriers. See ComputeComponentInstance::onCompute().
			flock_system.compute();

			// End the compute recording
			mRenderService->endComputeRecording();
		}
		
		// Begin recording the render commands for the offscreen render target. Rendering always happens after compute.
		// This prepares a command buffer and starts a render pass.
		if (mRenderService->beginHeadlessRecording())
		{
			// The world entity holds all visible renderable components in the scene.
			std::vector<RenderableComponentInstance*> render_comps;
			mWorldEntity->getComponentsOfTypeRecursive<RenderableComponentInstance>(render_comps);

			// Offscreen color pass -> Render all available geometry to the color texture bound to the render target.
			mRenderTarget->beginRendering();
			mRenderService->renderObjects(*mRenderTarget, mCameraEntity->getComponent<PerspCameraComponentInstance>(), render_comps);	
			mRenderTarget->endRendering();

			// Offscreen contrast pass -> Use previous `ColorTexture` as input, `ColorTextureFX` as output.
			// Input and output resources of these operations are described in JSON in their appropriate components.
			mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("ChangeColor")->draw();

			// Offscreen bloom pass -> Use `ColorTextureFX` as input and output.
			// This is fine as the bloom component blits the input to internally managed render targets on which the effect is applied.
			// the effect result is blitted to the output texture. The effect therefore does not write to itself.
			mRenderEntity->getComponent<RenderBloomComponentInstance>().draw();

			// End headless recording
			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window. Here we composite the color and effect texture with a
		// custom shader, and render the result to the window using a nap::RenderToTextureComponentInstance.
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get composite component responsible for rendering final texture
			auto* composite_comp = mRenderEntity->findComponentByID<RenderToTextureComponentInstance>("BlendTogether");

			// Render composite component
			// The nap::RenderToTextureComponentInstance transforms a plane to match the window dimensions and applies the texture to it.
			mRenderService->renderObjects(*mRenderWindow, mOrthoCameraEntity->getComponent<OrthoCameraComponentInstance>(), { composite_comp });

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
