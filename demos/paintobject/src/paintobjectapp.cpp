/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "paintobjectapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <uniforminstance.h>
#include <imguiutils.h>
#include <rendertotexturecomponent.h>
#include <triangleiterator.h>
#include <meshutils.h>
#include <mathutils.h>
#include <uniforminstance.h>
#include <renderglobals.h>
#include <orbitcontroller.h>
#include <cameracontroller.h>


// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PaintObjectApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool PaintObjectApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService		= getCore().getService<nap::RenderService>();
		mSceneService		= getCore().getService<nap::SceneService>();
		mInputService		= getCore().getService<nap::InputService>();
		mGuiService			= getCore().getService<nap::IMGuiService>();
		mParameterService	= getCore().getService<nap::ParameterService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("paintobject.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow			= mResourceManager->findObject<nap::RenderWindow>("Window0");
		mPaintTexture			= mResourceManager->findObject<nap::RenderTexture2D>("PaintRenderTexture");
		mBrushTexture			= mResourceManager->findObject<nap::RenderTexture2D>("BrushRenderTexture");
		mBrushColorParam		= mResourceManager->findObject<nap::ParameterRGBColorFloat>("BrushColorParam");
		mBrushSizeParam			= mResourceManager->findObject<nap::ParameterFloat>("BrushSizeParam");
		mBrushSoftnessParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushSoftnessParam");
		mBrushFalloffParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushFallOffParam");
		mLightIntensityParam	= mResourceManager->findObject<nap::ParameterFloat>("LightIntensityParam");
		mEraserModeParam		= mResourceManager->findObject<nap::ParameterBool>("EraserModeParam");
		mMeshSelectionParam		= mResourceManager->findObject<nap::ParameterInt>("MeshSelectionParam");
		mParameterGroup			= mResourceManager->findObject<nap::ParameterGroup>("Parameters");

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Fetch entities
		mWorldEntity	= scene->findEntity("World");
		mBrushEntity	= scene->findEntity("Brush");
		mLightEntity	= scene->findEntity("LightEntity");

		// Fetch the perspective camera
		mPerspectiveCamEntity	= scene->findEntity("PerspectiveCamera");

		// Start in draw mode, so disable orbit controller
		OrbitControllerInstance& orbit_controller = mPerspectiveCamEntity->getComponent<OrbitControllerInstance>();
		orbit_controller.disable();

		// Begin with pigmesh as object to paint on
		mSelectedMeshRendererID = "PigRenderer";

		// Create parameter GUI, used to draw parameters
		mParameterGUI = std::make_unique<ParameterGUI>(*mParameterService);

		return true;
	}


	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 *
	 * The camera has two input components: KeyInputComponent and PointerInputComponent
	 * The key input component receives key events, the pointer input component receives pointer events
	 * The orbit controller listens to both of them
	 * When an input component receives a message it sends a signal to the orbit controller.
	 * The orbit controller validates if it's something useful and acts accordingly,
	 * in this case by rotating around or zooming in on the sphere.
	 */
	void PaintObjectApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Add some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "Hold LMB to spray paint on object");
		ImGui::TextColored(clr, "Hold space + LMB to rotate, RMB to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		// Mesh Selection
		if (ImGui::SliderInt(mMeshSelectionParam->mName.c_str(), &mMeshSelectionParam->mValue, mMeshSelectionParam->mMinimum, mMeshSelectionParam->mMaximum))
			switchMesh(mMeshSelectionParam->mValue);

		// Light Intensity
		ImGui::SliderFloat(mLightIntensityParam->mName.c_str(), &mLightIntensityParam->mValue, mLightIntensityParam->mMinimum, mLightIntensityParam->mMaximum);

		// Serializable parameters (presets)
		if(ImGui::CollapsingHeader("Paint"))
			mParameterGUI->show(mParameterGroup.get(), false);

		// Display render textures
		if (ImGui::CollapsingHeader("Textures"))
		{
			float col_width = ImGui::GetColumnWidth();
			float ratio = (float)mPaintTexture->getHeight() / (float)mPaintTexture->getWidth();
			ImGui::Text("Paint Texture");
			ImGui::Image(*mPaintTexture, ImVec2(col_width, col_width * ratio));
			ImGui::Text("Brush Texture");
			ImGui::Image(*mBrushTexture, ImVec2(col_width, col_width * ratio));
		}

		ImGui::End();
	}

	/**
	 * Renders the procedurally generated brush texture which is used to draw the paint
	 * Should be called after beginHeadlessRecording() and before endHeadlessRecording() on the render service
	 */
	void PaintObjectApp::renderBrush()
	{
		// Get the brush render to texture component instance
		auto& brush_renderer = mBrushEntity->getComponent<nap::RenderToTextureComponentInstance>();

		// Set falloff and softness uniforms
		auto* ubo = brush_renderer.getMaterialInstance().getOrCreateUniform("UBO");
		auto* softness = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inSoftness");
		softness->setValue(mBrushSoftnessParam->mValue);
		auto* falloff = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inFalloff");
		falloff->setValue(mBrushFalloffParam->mValue);

		// Draw the brush
		brush_renderer.draw();
	}

	/*
	 * Renders the paint texture using the rendered brush
	 * Should be called after beginHeadlessRecording() and before endHeadlessRecording() on the render service
	 */
	void PaintObjectApp::renderPaint()
	{
		// Get the render to texture component
		auto& render_to_texture = mWorldEntity->getComponent<nap::RenderToTextureComponentInstance>();

		// Fetch the ubo struct uniform
		auto* ubo = render_to_texture.getMaterialInstance().getOrCreateUniform("UBO");

		// Set the brush color
		auto* brush_color = ubo->getOrCreateUniform<nap::UniformVec4Instance>("inBrushColor");
		auto col = mBrushColorParam->getValue();
		brush_color->setValue({ col.toVec3(), 1.0f });

		// Give mouse position in UV space
		auto* mouse_pos = ubo->getOrCreateUniform<nap::UniformVec2Instance>("inMousePosition");
		mouse_pos->setValue(mMousePosOnObject);

		// Set brush size
		nap::UniformFloatInstance* brush_size = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inBrushSize");
		brush_size->setValue(mBrushSizeParam->mValue);

		// Set eraser mode
		nap::UniformFloatInstance* eraser_mode = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inEraserAmount");
		eraser_mode->setValue((float)mEraserModeParam->mValue);

		// Set final multiplier, used to clear the paint texture
		nap::UniformFloatInstance* final_multiplier = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inFinalMultiplier");
		final_multiplier->setValue(1.0f);

		// Draw the render texture
		render_to_texture.draw();
	}

	/**
	 * Removes all paint from paint texture in one pass
	 * This is done by setting the inFinalMultiplier float uniform to zero, which renders all pixels in the render texture black with no alpha
	 * Should be called after beginHeadlessRecording() and before endHeadlessRecording() on the render service
	 */
	void PaintObjectApp::removeAllPaint()
	{
		// Get the render to texture component
		auto& render_to_texture = mWorldEntity->getComponent<nap::RenderToTextureComponentInstance>();

		// Get the ubo struct uniform
		auto* ubo = render_to_texture.getMaterialInstance().getOrCreateUniform("UBO");

		// Get the final multiplier uniform
		// Set it to zero, so we will render texture with all pixels set to 0, 0, 0, 0 rgba
		auto* final_multiplier = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inFinalMultiplier");
		final_multiplier->setValue(0.0f);

		// Draw into the render texture
		render_to_texture.draw();
		mClearPaint = false;
	}


	/**
	 * Render loop is rather straight forward:
	 * Render the brush texture that is used to render the paint
	 * Then render the paint on the UV position of the mouse on the object
	 * Use the rendered paint texture, together with the light and camera position, to render the object
	 */
	void PaintObjectApp::render()
	{
		// Let the render service now we are beginning to render to off-screen buffers
		if (mRenderService->beginHeadlessRecording())
		{
			// Clear paint if necessary
			if( mClearPaint )
				removeAllPaint();

			// Render new paint if necessary
			if (mDrawMode && mMouseOnObject && mMouseDown)
			{
				// First, render the brush
				renderBrush();

				// Now, render the new paint
				renderPaint();
			}

			// Let the render service now we are finished rendering to off-screen buffers
			mRenderService->endHeadlessRecording();
		}

		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Find the world and add as an object to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			nap::RenderableMeshComponentInstance& renderable_mesh = *mWorldEntity->findComponentByID<nap::RenderableMeshComponentInstance>(mSelectedMeshRendererID);
			components_to_render.emplace_back(&renderable_mesh);

			// Update the material of the mesh that renders the object
			nap::UniformStructInstance* ubo = renderable_mesh.getMaterialInstance().getOrCreateUniform("UBO");

			// Set light position uniform
			nap::UniformVec3Instance* light_pos_uniform = ubo->getOrCreateUniform<UniformVec3Instance>("LightPosition");
			nap::TransformComponentInstance& light_transform = mLightEntity->getComponent<nap::TransformComponentInstance>();
			light_pos_uniform->setValue(math::extractPosition(light_transform.getGlobalTransform()));

			// Set light intensity uniform
			nap::UniformFloatInstance* light_intensity_uniform = ubo->getOrCreateUniform<UniformFloatInstance>("LightIntensity");
			light_intensity_uniform->setValue(mLightIntensityParam->mValue);

			// Find the perspective camera
			nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

			// Render the world with the right camera directly to screen
			mRenderService->renderObjects(*mRenderWindow, persp_camera, components_to_render);

			// Draw our GUI
			mGuiService->draw();

			// End the render pass
			mRenderWindow->endRendering();

			// End recording
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
	void PaintObjectApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Input event handling, can also be done in a separate component but for
	 * this demo handling it directly inside the application is sufficient. 
	 */
	void PaintObjectApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
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

			// If 'c' is pressed, clear all paint on painted render texture
			if (press_event->mKey == nap::EKeyCode::KEY_c)
			{
				mClearPaint = true;
			}

			// If 'space' is down, move the camera with the mouse instead of painting on object
			if (press_event->mKey == nap::EKeyCode::KEY_SPACE)
			{
				mDrawMode = false;

				OrbitControllerInstance& orbit_controller = mPerspectiveCamEntity->getComponent<OrbitControllerInstance>();
				TransformComponentInstance& world_xform = mWorldEntity->getComponent<TransformComponentInstance>();
				orbit_controller.enable(world_xform.getTranslate());
			}
		} 
		
		// if 'space' is released, go back to paint mode and stop the mouse from moving the camera
		else if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyReleaseEvent)))
		{
			nap::KeyReleaseEvent* release_event = static_cast<nap::KeyReleaseEvent*>(inputEvent.get());
			if (release_event->mKey == nap::EKeyCode::KEY_SPACE)
			{
				mDrawMode = true;

				OrbitControllerInstance& orbit_controller = mPerspectiveCamEntity->getComponent<OrbitControllerInstance>();
				orbit_controller.disable();
			}
		}

		// Check for mouse down
		else if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			nap::PointerPressEvent* event = static_cast<nap::PointerPressEvent*>(inputEvent.get());
			if (event->mButton == EMouseButton::LEFT)
			{
				mMouseDown = true;
			}
		}

		// Check for mouse release
		else if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerReleaseEvent)))
		{
			nap::PointerReleaseEvent* event = static_cast<nap::PointerReleaseEvent*>(inputEvent.get());
			if (event->mButton == EMouseButton::LEFT)
			{
				mMouseDown = false;
			}
		}

		// If mouse is down and draw mode is on, perform trace
		if (mMouseDown && mDrawMode)
		{
			// Perform trace when the mouse is down
			if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerMoveEvent)) ||
				inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
			{
				nap::PointerEvent* event = static_cast<nap::PointerEvent*>(inputEvent.get());
				doTrace(*event);
			}
		}
		else
		{
			mMouseOnObject = false;
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int PaintObjectApp::shutdown()
	{
		return 0;
	}


	/**
	 * Performs a raycast and looks for any intersecting triangles
	 * When intersection occurs, lookup the UV coordinate of the mouse position on the object
	 * This will be the position we use to add paint in UV space
	 * @param event the pointer event
	 */
	void PaintObjectApp::doTrace(const PointerEvent& event)
	{
		// If we're not drawing and mouse is not down, skip
		if (!(mDrawMode && mMouseDown))
			mMouseOnObject = false;

		// Get the camera and camera transform
		PerspCameraComponentInstance& camera = mPerspectiveCamEntity->getComponent<PerspCameraComponentInstance>();
		TransformComponentInstance& camera_xform = mPerspectiveCamEntity->getComponent<TransformComponentInstance>();

		// Get screen (window) location from mouse event
		glm::ivec2 screen_loc = { event.mX, event.mY };

		// Get object to world transformation matrix
		TransformComponentInstance& world_xform = mWorldEntity->getComponent<TransformComponentInstance>();

		// Get the attributes we need, the vertices (position data) is used to perform a world space triangle intersection
		// The uv attribute is to compute the uv coordinates when a triangle is hit
		MeshInstance& mesh = mWorldEntity->findComponentByID<nap::RenderableMeshComponentInstance>(mSelectedMeshRendererID)->getMeshInstance();
		VertexAttribute<glm::vec3>& vertices = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
		VertexAttribute<glm::vec3>& uvs = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));

		// Get ray from screen in to scene (world space)
		// The result is a normal pointing away from the camera in to the scene
		// The window is used to provide the viewport
		glm::vec3 screen_to_world_ray = camera.rayFromScreen(screen_loc, mRenderWindow->getRect());

		// World space camera position
		glm::vec3 cam_pos = math::extractPosition(camera_xform.getGlobalTransform());

		// Used by intersection call
		TriangleData<glm::vec3> tri_vertices;

		// Create the triangle iterator
		TriangleIterator triangle_it(mesh);

		// Perform intersection test, walk over every triangle in the mesh
		mMouseOnObject = false;
		while (!triangle_it.isDone())
		{
			// Use the indices to get the vertex positions
			Triangle triangle = triangle_it.next();

			tri_vertices[0] = (math::objectToWorld(vertices[triangle[0]], world_xform.getGlobalTransform()));
			tri_vertices[1] = (math::objectToWorld(vertices[triangle[1]], world_xform.getGlobalTransform()));
			tri_vertices[2] = (math::objectToWorld(vertices[triangle[2]], world_xform.getGlobalTransform()));

			glm::vec3 bary_coord;
			if (utility::intersect(cam_pos, screen_to_world_ray, tri_vertices, bary_coord))
			{
				TriangleData<glm::vec3> uv_triangle_data = triangle.getVertexData(uvs);
				mMousePosOnObject = utility::interpolateVertexAttr<glm::vec3>(uv_triangle_data, bary_coord);
				mMouseOnObject = true;
				break;
			}
		}
	}

	/**
	 * Switch mesh on which we want to paint
	 * @param selection integer between 0-3
	 */
	void PaintObjectApp::switchMesh(int selection)
	{
		switch(selection)
		{
		case 0:
			mSelectedMeshRendererID = "PigRenderer";
			break;
		case 1:
			mSelectedMeshRendererID = "WorldRenderer";
			break;
		case 2:
			mSelectedMeshRendererID = "BearRenderer";
			break;
		}

		mClearPaint = true;
	}
}
