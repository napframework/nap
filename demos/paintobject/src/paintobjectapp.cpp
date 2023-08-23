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
	// ImGUI debug popup ID
	static constexpr const char* popupID = "Running Debug Build";

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

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mRenderWindow			= mResourceManager->findObject<nap::RenderWindow>("Window0");
		mPaintTextureA			= mResourceManager->findObject<nap::RenderTexture2D>("PaintRenderTextureA");
		mPaintTextureB			= mResourceManager->findObject<nap::RenderTexture2D>("PaintRenderTextureB");
		mBrushTexture			= mResourceManager->findObject<nap::RenderTexture2D>("BrushRenderTexture");
		mBrushColorParam		= mResourceManager->findObject<nap::ParameterRGBColorFloat>("BrushColorParam");
		mBrushSizeParam			= mResourceManager->findObject<nap::ParameterFloat>("BrushSizeParam");
		mBrushSoftnessParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushSoftnessParam");
		mBrushFalloffParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushFallOffParam");
		mLightIntensityParam	= mResourceManager->findObject<nap::ParameterFloat>("LightIntensityParam");
		mEraserModeParam		= mResourceManager->findObject<nap::ParameterBool>("EraserModeParam");
		mMeshSelectionParam		= mResourceManager->findObject<nap::ParameterInt>("MeshSelectionParam");
		mParameterGUI			= mResourceManager->findObject<ParameterGUI>("ParameterGUI");

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
		std::vector<EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Add some gui elements
		ImGui::Begin("Controls");

		// Notify user that painting in debug mode is slow
#ifndef NDEBUG
		static bool popup_opened = false;
		if (!popup_opened)
		{
			ImGui::OpenPopup(popupID);
			popup_opened = true;
		}
		handlePopup();
#endif // DEBUG

		ImGui::Text(getCurrentDateTime().toString().c_str());
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor1, "Hold LMB to spray paint on object");
		ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "Hold space + LMB to rotate, RMB to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

		// Mesh Selection
		if (ImGui::SliderInt(mMeshSelectionParam->mName.c_str(), &mMeshSelectionParam->mValue, mMeshSelectionParam->mMinimum, mMeshSelectionParam->mMaximum))
			switchMesh(mMeshSelectionParam->mValue);

		// Light Intensity
		ImGui::SliderFloat(mLightIntensityParam->mName.c_str(), &mLightIntensityParam->mValue, mLightIntensityParam->mMinimum, mLightIntensityParam->mMaximum);

		// Serializable parameters (presets)
		if(ImGui::CollapsingHeader("Paint"))
			mParameterGUI->show(false);

		// Display render textures
		if (ImGui::CollapsingHeader("Textures"))
		{
			float col_width = ImGui::GetColumnWidth();
			float ratio = (float)mPaintTextureA->getHeight() / (float)mPaintTextureA->getWidth();
			ImGui::Text("Brush Texture");
			ImGui::Image(*mBrushTexture, ImVec2(col_width, col_width * ratio));
			ImGui::Text("Paint Texture");
			ImGui::Image(*mPaintTextureA, ImVec2(col_width, col_width * ratio));
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
		auto& brush_renderer = mBrushEntity->getComponent<RenderToTextureComponentInstance>();

		// Set brush uniforms
		auto* brush_ubo = brush_renderer.getMaterialInstance().getOrCreateUniform("UBO");
		auto* brush_softness = brush_ubo->getOrCreateUniform<UniformFloatInstance>("inSoftness");
		brush_softness->setValue(mBrushSoftnessParam->mValue);
		auto* brush_falloff = brush_ubo->getOrCreateUniform<UniformFloatInstance>("inFalloff");
		brush_falloff->setValue(mBrushFalloffParam->mValue);

		// Draw the brush
		brush_renderer.draw();
	}

	/*
	 * Renders the paint texture using the rendered brush
	 * Should be called after beginHeadlessRecording() and before endHeadlessRecording() on the render service
	 */
	void PaintObjectApp::renderPaint()
	{
		// Only render paint when it is being applied or needs to be removed
		if (!isPainting() && !mClearPaint)
			return;

		// Get the component that can draw the paint effect.
		nap::EntityInstance& paint_entity = (*mWorldEntity)[mPaintIndex];
		auto& paint_to_texture = paint_entity.getComponent<RenderToTextureComponentInstance>();

		// Get the struct that contains all the uniforms we want to set
		auto* paint_ubo = paint_to_texture.getMaterialInstance().getOrCreateUniform("UBO");

		// Set the brush color
		auto* brush_color = paint_ubo->getOrCreateUniform<UniformVec4Instance>("inBrushColor");
		glm::vec3 col = mBrushColorParam->getValue().convert<RGBColorFloat>().toVec3();
		brush_color->setValue({ col, 1.0f });
		
		// Give mouse position in UV space
		auto* mouse_pos = paint_ubo->getOrCreateUniform<UniformVec2Instance>("inMousePosition");
		mouse_pos->setValue(mMousePosOnObject);

		// Set brush size
		UniformFloatInstance* brush_size = paint_ubo->getOrCreateUniform<UniformFloatInstance>("inBrushSize");
		brush_size->setValue(mBrushSizeParam->mValue);

		// Set eraser mode
		UniformFloatInstance* eraser_mode = paint_ubo->getOrCreateUniform<UniformFloatInstance>("inEraserAmount");
		eraser_mode->setValue((float)mEraserModeParam->mValue);

		// Set final multiplier, used to clear the paint texture
		UniformFloatInstance* final_multiplier = paint_ubo->getOrCreateUniform<UniformFloatInstance>("inFinalMultiplier");
		final_multiplier->setValue(mClearPaint ? 0.0f : 1.0f);
		mClearPaint = false;

		// Draw the render texture
		paint_to_texture.draw();

		// Ensure the mesh that is rendered uses the texture we just painted into
		RenderableMeshComponentInstance& renderable_mesh = *mWorldEntity->findComponentByID<RenderableMeshComponentInstance>(mSelectedMeshRendererID);
		Sampler2DInstance* sampler = renderable_mesh.getMaterialInstance().getOrCreateSampler<Sampler2DInstance>("inPaintTexture");
		sampler->setTexture(paint_to_texture.getOutputTexture());

		// Increment paint index. We do this to avoid a feedback loop, where
		// the render texture is both the target and source, which isn't allowed by Vulkan.
		// To counter this we use bounce passes:
		// Frame 1: Read from texture A, render into texture B
		// Frame 2: Read from texture B, render into texture A
		// Frame 3: Read from texture A, render into texture B
		// etc.
		mPaintIndex = ++mPaintIndex % 2;
	}


	/**
	 * Render loop is rather straight forward:
	 * Render the brush texture that is used to render the paint
	 * Then render the paint on the UV position of the mouse on the object
	 * Use the rendered paint texture, together with the light and camera position, to render the object
	 */
	void PaintObjectApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Let the render service now we are beginning to render to off-screen buffers
		if (mRenderService->beginHeadlessRecording())
		{
			// Render brush and paint
			renderBrush();
			renderPaint();

			// Let the render service now we are finished rendering to off-screen buffers
			mRenderService->endHeadlessRecording();
		}

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Find the world and add as an object to render
			std::vector<RenderableComponentInstance*> components_to_render;
			RenderableMeshComponentInstance& renderable_mesh = *mWorldEntity->findComponentByID<RenderableMeshComponentInstance>(mSelectedMeshRendererID);
			components_to_render.emplace_back(&renderable_mesh);

			// Update the material of the mesh that renders the object
			UniformStructInstance* ubo = renderable_mesh.getMaterialInstance().getOrCreateUniform("UBO");

			// Set light position uniform
			UniformVec3Instance* light_pos_uniform = ubo->getOrCreateUniform<UniformVec3Instance>("LightPosition");
			TransformComponentInstance& light_transform = mLightEntity->getComponent<TransformComponentInstance>();
			light_pos_uniform->setValue(math::extractPosition(light_transform.getGlobalTransform()));

			// Set light intensity uniform
			UniformFloatInstance* light_intensity_uniform = ubo->getOrCreateUniform<UniformFloatInstance>("LightIntensity");
			light_intensity_uniform->setValue(mLightIntensityParam->mValue);

			// Find the perspective camera
			PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<PerspCameraComponentInstance>();

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
			if (event->mButton == PointerClickEvent::EButton::LEFT)
			{
				mMouseDown = true;
			}
		}

		// Check for mouse release
		else if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerReleaseEvent)))
		{
			nap::PointerReleaseEvent* event = static_cast<nap::PointerReleaseEvent*>(inputEvent.get());
			if (event->mButton == PointerClickEvent::EButton::LEFT)
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
	 * Performs a ray-cast and looks for any intersecting triangles
	 * When intersection occurs, lookup the UV coordinate of the mouse position on the object
	 * This will be the position we use to add paint in UV space.
	 * Depending on the complexity of the geometry this call can be slow, as it currently iterates
	 * over all triangles until it finds a possible match. Can be optimized using faster indexing methods such
	 * as a KD-tree etc. 
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


	bool PaintObjectApp::isPainting() const
	{
		return mDrawMode && mMouseOnObject && mMouseDown;
	}


	void PaintObjectApp::handlePopup()
	{
		if (ImGui::BeginPopupModal(popupID, nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Painting is slow in a debug build");
			ImGui::SameLine();
			if (ImGui::ImageButton(mGuiService->getIcon(icon::ok), "Gotcha"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}
