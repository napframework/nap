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
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("paintobject.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow			= mResourceManager->findObject<nap::RenderWindow>("Window0");
		mPaintTexture			= mResourceManager->findObject<nap::RenderTexture2D>("PaintTexture");
		mBrushTexture			= mResourceManager->findObject<nap::RenderTexture2D>("BrushTexture");
		mBrushColorParam		= mResourceManager->findObject<nap::ParameterRGBColorFloat>("BrushColorParam");
		mBrushSizeParam			= mResourceManager->findObject<nap::ParameterFloat>("Brush Size");
		mBrushSoftnessParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushSoftnessParam");
		mBrushFalloffParam		= mResourceManager->findObject<nap::ParameterFloat>("BrushFallOffParam");
		mLightIntensityParam	= mResourceManager->findObject<nap::ParameterFloat>("LightIntensityParam");
		mEraserModeParam		= mResourceManager->findObject<nap::ParameterBool>("EraserModeParam");

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
		ImGui::TextColored(clr, "hold left mouse button to spray paint on object");
		ImGui::TextColored(clr, "Hold spacebar + left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

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

		// Display brush and paint parameters
		if (ImGui::CollapsingHeader("Parameters"))
		{
			// Brush parameters
			ImGui::SliderFloat("Brush Softness", &mBrushSoftnessParam->mValue, mBrushSoftnessParam->mMinimum, mBrushSoftnessParam->mMaximum);
			ImGui::SliderFloat("Brush Falloff", &mBrushFalloffParam->mValue, mBrushFalloffParam->mMinimum, mBrushFalloffParam->mMaximum);
			ImGui::SliderFloat("Brush Size", &mBrushSizeParam->mValue, 0.001f, 1.0f);

			// Eraser mode
			ImGui::Checkbox("Eraser mode", &mEraserModeParam->mValue);

			// Brush color
			glm::vec3 color(mBrushColorParam->mValue.getRed(), mBrushColorParam->mValue.getGreen(), mBrushColorParam->mValue.getBlue());
			if (ImGui::ColorPicker3("Brush Color", &color.r))
			{
				mBrushColorParam->setValue(RGBColorFloat(color.r, color.g, color.b));
			}

			// Light intensity parameter
			ImGui::SliderFloat("Light Intensity", &mLightIntensityParam->mValue, mLightIntensityParam->mMinimum, mLightIntensityParam->mMaximum);
		}

		ImGui::End();
	}


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

		// Start a new frame to start headless recording
		mRenderService->beginFrame();

		// Let the renderservice now we start a headless recording
		if (mRenderService->beginHeadlessRecording())
		{
			// Draw the brush
			brush_renderer.draw();

			// End headless recording
			mRenderService->endHeadlessRecording();
		}

		// End frame
		mRenderService->endFrame();
	}


	void PaintObjectApp::renderPaint()
	{
		// Get the render to texture component
		auto& render_to_texture = mWorldEntity->getComponent<nap::RenderToTextureComponentInstance>();

		// Fetch the ubo struct uniform
		auto* ubo = render_to_texture.getMaterialInstance().getOrCreateUniform("UBO");

		// Set the brush color
		auto* brush_color = ubo->getOrCreateUniform<nap::UniformVec4Instance>("inBrushColor");
		auto col = mBrushColorParam->getValue();
		brush_color->setValue(glm::vec4(col.getRed(), col.getGreen(), col.getBlue(), 1.0f));

		// Give mouse position in UV space
		auto* mouse_pos = ubo->getOrCreateUniform<nap::UniformVec2Instance>("inMousePosition");
		mouse_pos->setValue(mMousePosOnObject);

		// Set brush size
		nap::UniformFloatInstance* brush_size = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inBrushSize");
		brush_size->setValue(mBrushSizeParam->mValue);

		// Set eraser mode
		nap::UniformFloatInstance* eraser_mode = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inEraserAmount");
		eraser_mode->setValue((float)mEraserModeParam->mValue);

		// Set alpha multiplier, used to clear the paint texture
		nap::UniformFloatInstance* alpha_multiplier = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inAlphaMultiplier");
		alpha_multiplier->setValue(1.0f);

		// Let the renderservice know we begin a new frame
		mRenderService->beginFrame();

		// Begin headless recording
		if (mRenderService->beginHeadlessRecording())
		{
			// Draw the render texture
			render_to_texture.draw();

			// End headless recording
			mRenderService->endHeadlessRecording();
		}

		// End rendering
		mRenderService->endFrame();
	}

	void PaintObjectApp::removeAllPaint()
	{
		// Get the render to texture component
		auto& render_to_texture = mWorldEntity->getComponent<nap::RenderToTextureComponentInstance>();

		// Get the ubo struct uniform
		auto* ubo = render_to_texture.getMaterialInstance().getOrCreateUniform("UBO");

		// Get the alpha multiplier uniform
		// Set it to zero, so we will render texture with all pixels set to 0, 0, 0, 0 rgba
		auto* alpha_multiplier = ubo->getOrCreateUniform<nap::UniformFloatInstance>("inAlphaMultiplier");
		alpha_multiplier->setValue(0.0f);

		// Let the renderservice know we begin a new frame
		mRenderService->beginFrame();

		// Start recording
		if (mRenderService->beginHeadlessRecording())
		{
			// Draw into the render texture
			render_to_texture.draw();

			// End recording
			mRenderService->endHeadlessRecording();
		}

		// End rendering
		mRenderService->endFrame();
	}


	/**
	 * Render loop is rather straight forward:
	 * Render the brush texture that is used to render the paint
	 * Then render the paint on the UV position of the mouse on the object
	 * Use the rendered paint texture, together with the light and camera position, to render the object
	 */
	void PaintObjectApp::render()
	{
		// Render new paint if necessary
		if (mDrawMode && mMouseOnObject && mMouseDown)
		{
			// First, render the brush
			renderBrush();

			// Now, render the new paint
			renderPaint();
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
			nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
			components_to_render.emplace_back(&renderable_world);

			// Update the camera location in the world shader for the halo effect
			// To do that we fetch the material associated with the world mesh and query the camera location uniform
			// Once we have the uniform we can set it to the camera world space location
			nap::RenderableMeshComponentInstance& render_mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
			nap::UniformStructInstance* ubo = render_mesh.getMaterialInstance().getOrCreateUniform("UBO");

			// Give light position uniform
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
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
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

			if (press_event->mKey == nap::EKeyCode::KEY_c)
			{
				removeAllPaint();
			}

			if (press_event->mKey == nap::EKeyCode::KEY_SPACE)
			{
				mDrawMode = false;

				OrbitControllerInstance& orbit_controller = mPerspectiveCamEntity->getComponent<OrbitControllerInstance>();
				TransformComponentInstance& world_xform = mWorldEntity->getComponent<TransformComponentInstance>();
				orbit_controller.enable(world_xform.getTranslate());
			}
		}
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

		//
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			nap::PointerPressEvent* event = static_cast<nap::PointerPressEvent*>(inputEvent.get());
			if (event->mButton == EMouseButton::LEFT)
			{
				mMouseDown = true;
			}
		}

		//
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerReleaseEvent)))
		{
			nap::PointerReleaseEvent* event = static_cast<nap::PointerReleaseEvent*>(inputEvent.get());
			if (event->mButton == EMouseButton::LEFT)
			{
				mMouseDown = false;
			}
		}

		if (mMouseDown)
		{
			// Perform trace when the mouse isn't down
			if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerMoveEvent)) ||
				inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
			{
				nap::PointerEvent* event = static_cast<nap::PointerEvent*>(inputEvent.get());
				doTrace(*event);
			}

			mInputService->addEvent(std::move(inputEvent));
		}
		else
		{
			mMouseOnObject = false;
		}
	}


	int PaintObjectApp::shutdown()
	{
		return 0;
	}


	void PaintObjectApp::doTrace(const PointerEvent& event)
	{
		// Get the camera and camera transform
		PerspCameraComponentInstance& camera = mPerspectiveCamEntity->getComponent<PerspCameraComponentInstance>();
		TransformComponentInstance& camera_xform = mPerspectiveCamEntity->getComponent<TransformComponentInstance>();

		// Get screen (window) location from mouse event
		glm::ivec2 screen_loc = { event.mX, event.mY };

		// Get object to world transformation matrix
		TransformComponentInstance& world_xform = mWorldEntity->getComponent<TransformComponentInstance>();

		// Get the attributes we need, the vertices (position data) is used to perform a world space triangle intersection
		// The uv attribute is to compute the uv coordinates when a triangle is hit
		MeshInstance& mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>().getMeshInstance();
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

		// Perform intersection test, walk over every triangle in the mesh.
		// In this case only 2, nice and fast. When there is a hit use the returned barycentric coordinates
		// to get the interpolated (triangulated) uv attribute value at point of intersection

		if (mDrawMode && mMouseDown)
		{
			bool onObject = false;
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
					onObject = true;
					break;
				}
			}

			mMouseOnObject = onObject;
		}
		else
		{
			mMouseOnObject = false;
		}
	}
}
