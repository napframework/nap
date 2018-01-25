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
#include <imgui/imgui.h>

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
		mHeightMesh = mResourceManager->findObject<nap::HeightMesh>("HeightMesh");
		mNormalsMaterial = mResourceManager->findObject<nap::Material>("NormalsMaterial");
		mHeightmapMaterial = mResourceManager->findObject<nap::Material>("HeightMaterial");

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

		// Find the world and camera entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mWorldEntity = scene->findEntity("World");
		mCameraEntity = scene->findEntity("Camera");

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
		mInputService->processEvents(*mRenderWindow, input_router, entities);

		// Update gui and check for gui changes
		updateGui();

		// get smoothed blend value
		float current_blend_value = mBlendSmoother.update(mBlendValue, deltaTime);
		
		// Now push the blend values to both materials
		float normal_blend_value = mBlendNormals ? current_blend_value : 0.0f;

		mNormalsMaterial->getUniform<UniformFloat>("blendValue").setValue(current_blend_value);
		mNormalsMaterial->getUniform<UniformFloat>("normalBlendValue").setValue(normal_blend_value);

		mHeightmapMaterial->getUniform<UniformFloat>("blendValue").setValue(current_blend_value);
		mHeightmapMaterial->getUniform<UniformFloat>("normalBlendValue").setValue(normal_blend_value);

		// Push all colors
		pushColor(mValleyColor, *mHeightmapMaterial, "lowerColor");
		pushColor(mPeakColor, *mHeightmapMaterial, "upperColor");
		pushColor(mHaloColor, *mHeightmapMaterial, "haloColor");
		pushColor(mNormalColor, *mNormalsMaterial, "color");

		// Set normal opacity
		mNormalsMaterial->getUniform<UniformFloat>("opacity").setValue(mNormalOpacity);

		// Set the normal length
		mNormalsMaterial->getUniform<UniformFloat>("length").setValue(mNormalLength);
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
		// Update the camera location in the world shader for the halo effect
		// To do that we fetch the material associated with the world mesh and query the camera location uniform
		// Once we have the uniform we can set it to the camera world space location
		nap::RenderableMeshComponentInstance& render_mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformVec3& cam_loc_uniform = render_mesh.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("inCameraPosition");

		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
		cam_loc_uniform.setValue(global_pos);

		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

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
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);

		// Render gui to window
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
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

	
	void HeightmapApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}


	int HeightmapApp::shutdown()
	{
		return 0;
	}


	void HeightmapApp::updateGui()
	{
		ImGui::Begin("Controls");
		ImGui::Checkbox("Blend Normals", &mBlendNormals);
		ImGui::Combo("Visualize", &mSelection, "Mesh\0Normals\0Both\0\0");
		ImGui::SliderFloat("Blend Value", &mBlendValue, 0.0f, 1.0f);
		ImGui::SliderFloat("Normal Length", &mNormalLength, 0.0f, 1.0f);
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


	void HeightmapApp::pushColor(RGBColorFloat& color, Material& material, const std::string& name)
	{
		// Push normal color
		glm::vec3 clr_data;
		clr_data.x = color.getRed();
		clr_data.y = color.getGreen();
		clr_data.z = color.getBlue();
		material.getUniform<UniformVec3>(name).setValue(clr_data);
	}

}
