#include "heightmapapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include<imgui/imgui.h>
#include <imguiutils.h>
#include <uniforminstances.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HeightmapApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool HeightmapApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("heightmap.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");		
		mHeightmap = mResourceManager->findObject<nap::ImageFromFile>("HeightMapTexture");

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mWorldEntity = scene->findEntity("World");
		mCameraEntity = scene->findEntity("Camera");

		// Select gui window
		mGuiService->selectWindow(mRenderWindow);

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 * 
	 * We also use the update to push all our colors and other values to the GPU
	 */
	void HeightmapApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update gui and check for gui changes
		updateGui();

		// get smoothed blend value
		float current_blend_value = mBlendSmoother.update(mBlendValue, deltaTime);
		
		// Now push the blend values to both materials
		float normal_blend_value = mBlendNormals ? current_blend_value : 0.0f;

		// First get a handle to the individual materials
		std::vector<nap::RenderableMeshComponentInstance*> heightmap_display_components;
		mWorldEntity->getComponentsOfType<nap::RenderableMeshComponentInstance>(heightmap_display_components);
		MaterialInstance& height_material = heightmap_display_components[0]->getMaterialInstance();
		MaterialInstance& normal_material = heightmap_display_components[1]->getMaterialInstance();

		// Get normal shader vertex UBO and update normal blend values
		UniformStructInstance* normal_vert_ubo = normal_material.getOrCreateUniform("VERTUBO");	
		normal_vert_ubo->getOrCreateUniform<UniformFloatInstance>("blendValue")->setValue(current_blend_value);
		normal_vert_ubo->getOrCreateUniform<UniformFloatInstance>("normalBlendValue")->setValue(normal_blend_value);

		// Get height shader vertex UBO and update normal blend values
		UniformStructInstance* height_vert_ubo = height_material.getOrCreateUniform("VERTUBO");
		height_vert_ubo->getOrCreateUniform<UniformFloatInstance>("blendValue")->setValue(current_blend_value);
		height_vert_ubo->getOrCreateUniform<UniformFloatInstance>("normalBlendValue")->setValue(normal_blend_value);

		// Push all colors
		pushColor(mValleyColor, height_material, "FRAGUBO", "lowerColor");
		pushColor(mPeakColor, height_material, "FRAGUBO", "upperColor");
		pushColor(mHaloColor, height_material, "FRAGUBO", "haloColor");
		pushColor(mNormalColor, normal_material, "FRAGUBO", "color");

		// Set normal opacity and length in normal material
		UniformStructInstance* normal_frag_ubo = normal_material.getOrCreateUniform("FRAGUBO");
		normal_frag_ubo->getOrCreateUniform<UniformFloatInstance>("opacity")->setValue(mNormalOpacity);
		normal_frag_ubo->getOrCreateUniform<UniformFloatInstance>("nlength")->setValue(mNormalLength);

		// Set camera position, first get handle to the camera position uniform
		UniformStructInstance* height_frag_ubo = height_material.getOrCreateUniform("FRAGUBO");
		nap::UniformVec3Instance* cam_loc_uniform = height_frag_ubo->getOrCreateUniform<nap::UniformVec3Instance>("inCameraPosition");

		// Extract world space position and set in material
		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
		cam_loc_uniform->setValue(global_pos);

		// Update blend state in fragment shader of for height material
		height_frag_ubo->getOrCreateUniform<UniformFloatInstance>("blendValue")->setValue(current_blend_value);
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
	void HeightmapApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Find the height map related objects (mesh / normals) and add as object to render
			// We make a selection based on the set we want to visualize
			std::vector<nap::RenderableComponentInstance*> heightmap_display_components;
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			mWorldEntity->getComponentsOfType<nap::RenderableComponentInstance>(heightmap_display_components);
			switch (mSelection)
			{
			case 0:
				components_to_render.emplace_back(heightmap_display_components[0]);
				break;
			case 1:
				components_to_render.emplace_back(heightmap_display_components[1]);
				break;
			case 2:
				components_to_render = heightmap_display_components;
				break;
			default:
				assert(false);
			}

			// Find the camera
			nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(*mRenderWindow, camera, components_to_render);

			// Render GUI to window
			mGuiService->draw();

			// End render pass
			mRenderWindow->endRendering();
			
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
	void HeightmapApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void HeightmapApp::inputMessageReceived(InputEventPtr inputEvent)
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
				mRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int HeightmapApp::shutdown()
	{
		return 0;
	}


	void HeightmapApp::updateGui()
	{
		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBColorFloat clr = mTextHighlightColor.convert<RGBColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		if (ImGui::CollapsingHeader("Blending"))
		{
			ImGui::Checkbox("Blend Normals", &mBlendNormals);
			ImGui::Combo("Visualize", &mSelection, "Mesh\0Normals\0Both\0\0");
			ImGui::SliderFloat("Blend Value", &mBlendValue, 0.0f, 1.0f);
			ImGui::SliderFloat("Normal Length", &mNormalLength, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Colors"))
		{
			ImGui::ColorEdit3("Valley Color", mValleyColor.getData());
			ImGui::ColorEdit3("Peak Color", mPeakColor.getData());
			ImGui::ColorEdit3("Halo Color", mHaloColor.getData());
			ImGui::ColorEdit3("Normal Color", mNormalColor.getData());
			ImGui::SliderFloat("Normal Opacity", &mNormalOpacity, 0.0f, 1.0f);
		}
		ImGui::End();
	}


	void HeightmapApp::pushColor(RGBColorFloat& color, MaterialInstance& material, const std::string& uboName, const std::string& uniformName)
	{
		UniformStructInstance* frag_ubo = material.getOrCreateUniform(uboName);
		frag_ubo->getOrCreateUniform<UniformVec3Instance>(uniformName)->setValue(color.toVec3());
	}

}
