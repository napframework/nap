/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Holoapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <rendertotexturecomponent.h>
#include <renderlightfieldcomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <uniforminstance.h>
#include <quiltrendertarget.h>
#include <quiltcameracomponent.h>
#include <focuscontroller.h>
#include <renderquiltcameraboxcomponent.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HoloApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool HoloApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService			= getCore().getService<nap::RenderService>();
		mSceneService			= getCore().getService<nap::SceneService>();
		mInputService			= getCore().getService<nap::InputService>();
		mGuiService				= getCore().getService<nap::IMGuiService>();
		mLookingGlassService	= getCore().getService<nap::LookingGlassService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

        // load resources from file
        if (!mResourceManager->loadFile("holo.json", error))
            return false;

        // Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("RenderWindow");
		mLookingGlassRenderWindow = mResourceManager->findObject<nap::RenderWindow>("LookingGlassRenderWindow");
		mParameterGroup = mResourceManager->findObject<nap::ParameterGroup>("Parameters");
		mParameterGUI = mResourceManager->findObject<ParameterGUI>("ParameterGUI");

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mWorldEntity = scene->findEntity("WorldEntity");
		mObjectEntity = scene->findEntity("ObjectEntity");
		mCameraEntity = scene->findEntity("CameraEntity");

		mWindowTextureEntity = scene->findEntity("WindowTextureEntity");
		mLookingGlassWindowTextureEntity = scene->findEntity("LookingGlassWindowTextureEntity");

		// Select gui window
		mGuiService->selectWindow(mRenderWindow);

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 */
	void HoloApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update the camera location in the world shader for the halo effect
		// To do that we fetch the material associated with the world mesh and query the camera location uniform
		// Once we have the uniform we can set it to the camera world space location
		nap::RenderableMeshComponentInstance& render_mesh = mObjectEntity->getComponent<nap::RenderableMeshComponentInstance>();

		// Get camera world space position and set
		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());

		nap::UniformStructInstance* ubo = render_mesh.getMaterialInstance().getOrCreateUniform("VERTUBO");
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("cameraLocation")->setValue(global_pos);

		ubo = render_mesh.getMaterialInstance().getOrCreateUniform("FRAGUBO");
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("cameraLocation")->setValue(global_pos);
		ubo->getOrCreateUniform<nap::UniformVec3Instance>("ambientColor")->setValue(mRenderWindow->getClearColor());

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		const RGBColorFloat clr = mTextHighlightColor.convert<RGBColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Performance: %.02f ms | %.02f fps", deltaTime*1000.0, getCore().getFramerate()).c_str());

		std::vector<nap::RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<nap::RenderableComponentInstance>(render_comps);

		ImGui::Separator();
		ImGui::Text("Renderable Components:");
		for (auto& comp : render_comps)
		{
			bool is_visible = comp->isVisible();
			if (ImGui::Checkbox(comp->mID.c_str(), &is_visible))
				comp->setVisible(is_visible);

			if (is_visible && comp->get_type() == RTTI_OF(RenderQuiltCameraBoxComponentInstance))
			{
				auto* box_comp = static_cast<RenderQuiltCameraBoxComponentInstance*>(comp);
				float depth = box_comp->getBoxDepth();
				const std::string label = utility::stringFormat("Frame depth", comp->mID.c_str());
				if (ImGui::SliderFloat(label.c_str(), &depth, 0.0f, 20.0f, "%.02f"))
					box_comp->setBoxDepth(depth);
			}
		}

		ImGui::Separator();
		ImGui::Text("Camera:");

		const glm::vec3& camera_position = math::extractPosition(cam_xform.getGlobalTransform());
		ImGui::Text("Position (%.02f, %.02f, %.02f) | Size %.02f",
			camera_position.x, camera_position.y, camera_position.z, mCameraEntity->getComponent<nap::QuiltCameraComponentInstance>().getCameraSize());

		nap::FocusControllerInstance& focus_controller = mCameraEntity->getComponent<nap::FocusControllerInstance>();
		bool auto_focus = focus_controller.isAutoFocusEnabled();
		if (ImGui::Checkbox("Autofocus", &auto_focus))
			focus_controller.setAutoFocus(auto_focus);

		ImGui::Separator();
		mParameterGUI->show(false);

		ImGui::End();
	}

	
	/**
	 * Render loop is rather straight forward:
	 * Set the camera position in the world shader for the halo effect. We do that here because the 
	 * transforms are updated after the app update() call.
	 * 
	 * We simply enable our window for drawing, clear it's buffers and draw our selection to screen
	 * You can select to draw only the mesh, only the normals or both combined
	 * The last step is always to draw the gui as it needs to be drawn on top of the rest
	 */
	void HoloApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// We make a selection based on the set we want to visualize
		std::vector<nap::RenderableComponentInstance*> render_comps;
		mWorldEntity->getComponentsOfTypeRecursive<nap::RenderableComponentInstance>(render_comps);

		auto quilt_render_targets = mResourceManager->getObjects<QuiltRenderTarget>();
		assert(!quilt_render_targets.empty());

		auto& quilt_target = quilt_render_targets.front();

		// Find the camera
		QuiltCameraComponentInstance& quilt_camera = mCameraEntity->getComponent<nap::QuiltCameraComponentInstance>();
		
		// Start recording into the headless recording buffer.
		if (mRenderService->beginHeadlessRecording())
		{
			// Render the world to a `nap::QuiltRenderTarget`. This requires a `nap::QuiltCameraComponentInstance`.
			// The function that is passed as an argument in repeated for each camera view to generate the quilt.
			quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](QuiltRenderTarget& target, QuiltCameraComponentInstance& camera)
			{
				// Render the world with the right camera directly to screen
				render_service->renderObjects(target, camera, comps);
			});

			// Tell the render service we are done rendering into render-targets.
			// The queue is submitted and executed.
			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window.
		// This window shows a preview of the quilt texture and the GUI.
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Get the `nap::RenderToTextureComponentInstance`.
			auto& rendertex_comp = mWindowTextureEntity->getComponent<RenderToTextureComponentInstance>();
			auto& ortho_camera = mWindowTextureEntity->getComponent<OrthoCameraComponentInstance>();

			// Render the quilt texture directly to the window
			mRenderService->renderObjects(*mRenderWindow, ortho_camera, { &rendertex_comp });

			// Render GUI to window
			mGuiService->draw();

			// End render pass
			mRenderWindow->endRendering();

			// Stop recording this render pass
			mRenderService->endRecording();
		}

		// Begin recording the render commands for the looking glass
		if (mRenderService->beginRecording(*mLookingGlassRenderWindow))
		{
			// Begin the render pass
			mLookingGlassRenderWindow->beginRendering();

			// Fetch the `nap::RenderLightFieldComponentInstance`.
			// This works very similar to `nap::RenderToTextureComponentInstance`, except that the color texture is drawn
			// to the rendertarget with a fully configured `nap::LightFieldShader`.
			auto& rendertex_comp = mLookingGlassWindowTextureEntity->getComponent<RenderLightFieldComponentInstance>();
			auto& ortho_camera = mLookingGlassWindowTextureEntity->getComponent<OrthoCameraComponentInstance>();

			// Render the `nap::RenderLightFieldComponentInstance` to the looking glass.
			mRenderService->renderObjects(*mLookingGlassRenderWindow, ortho_camera, {&rendertex_comp});

			// End render pass
			mLookingGlassRenderWindow->endRendering();
			 
			// Stop recording this render pass
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void HoloApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void HoloApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mLookingGlassRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int HoloApp::shutdown()
	{
		return 0;
	}
}
