#include "blobtraceapp.h"

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
#include <triangleiterator.h>
#include <meshutils.h>
#include <mathutils.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BlobTraceApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool BlobTraceApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("blobtrace.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Position window
		glm::ivec2 screen_size = opengl::getScreenSize(0);
		int offset_x = (screen_size.x - mRenderWindow->getWidth()) / 2;
		int offset_y = (screen_size.y - mRenderWindow->getHeight()) / 2;
		mRenderWindow->setPosition(glm::ivec2(offset_x, offset_y));

		// Find the camera and plane entities
		rtti::ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mPlaneEntity = scene->findEntity("Plane");
		mCameraEntity = scene->findEntity("Camera");

		// Find the mesh used for intersection testing
		mIntersectMesh = mResourceManager->findObject("IntersectMesh");

		return true;
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 * Set the shader uniforms. Time is scaled based on the current blob velocity
	 */
	void BlobTraceApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processEvents(*mRenderWindow, input_router, entities);

		// Set mouse pos in shader
		RenderableMeshComponentInstance& minstance = mPlaneEntity->getComponent<RenderableMeshComponentInstance>();
		UniformVec3& uv_uniform = minstance.getMaterialInstance().getOrCreateUniform<UniformVec3>("inBlobPosition");
		uv_uniform.setValue(mUVSmoother.update(mMouseUvPosition, deltaTime));

		// Set velocity in shader
		UniformFloat& vel_uniform = minstance.getMaterialInstance().getOrCreateUniform<UniformFloat>("inVelocity");
		float vel = math::fit(glm::length(mUVSmoother.getVelocity()), 0.0f, 0.66f, 0.0f,1.0f);
		vel_uniform.setValue(vel);

		// Set mouse position
		UniformVec3& mou_uniform = minstance.getMaterialInstance().getOrCreateUniform<UniformVec3>("inMousePosition");
		mou_uniform.setValue(mMouseUvPosition);

		// Set time in shader
		UniformFloat& time_uniform = minstance.getMaterialInstance().getOrCreateUniform<UniformFloat>("inTime");
		time_uniform.setValue(mTime);

		// Increment time based on velocity
		mTime += (deltaTime * math::fit<float>(vel, 0.0f, 1.0f, 1.0f, 3.0f));

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(utility::getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(ImVec4(clr.getRed(), clr.getGreen(), clr.getBlue(), clr.getAlpha()),
			"Move mouse over canvas to position blob\nLeft mouse button to rotate camera\nRight mouse button to zoom");
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		ImGui::End();
	}

	
	/**
	 * Render loop is rather straight forward:
	 * Set the current camera position in the shader for light calculations
	 * Render the plane using the perspective camera to screen
	 * Draw the gui and swap buffers
	 */
	void BlobTraceApp::render()
	{
		// Update the camera location in the world shader for the halo effect
		// To do that we fetch the material associated with the world mesh and query the camera location uniform
		// Once we have the uniform we can set it to the camera world space location
		nap::RenderableMeshComponentInstance& render_mesh = mPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>();
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

		// Find the world and add as an object to render
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		nap::RenderableMeshComponentInstance& renderable_world = mPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>();
		components_to_render.emplace_back(&renderable_world);

		// Find the camera
		nap::PerspCameraComponentInstance& camera = mCameraEntity->getComponent<nap::PerspCameraComponentInstance>();

		// Render the world with the right camera directly to screen
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), camera, components_to_render);

		// Draw our gui
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
	void BlobTraceApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void BlobTraceApp::inputMessageReceived(InputEventPtr inputEvent)
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
		}

		// Mouse down -> don't trace
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerPressEvent)))
		{
			mMouseDown = true;
		}

		// Mouse up -> allow tracing
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerReleaseEvent)))
		{
			mMouseDown = false;
		}

		// Perform trace when the mouse isn't down
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::PointerMoveEvent)) && !mMouseDown)
		{
			nap::PointerMoveEvent* event = static_cast<nap::PointerMoveEvent*>(inputEvent.get());
			doTrace(*event);
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int BlobTraceApp::shutdown()
	{
		return 0;
	}


	void BlobTraceApp::doTrace(const PointerEvent& event)
	{
		// Get the camera and camera transform
		PerspCameraComponentInstance& camera = mCameraEntity->getComponent<PerspCameraComponentInstance>();
		TransformComponentInstance& camera_xform = mCameraEntity->getComponent<TransformComponentInstance>();

		// Get screen (window) location from mouse event
		glm::ivec2 screen_loc = { event.mX, event.mY };

		// Get object to world transformation matrix
		TransformComponentInstance& world_xform = mPlaneEntity->getComponent<TransformComponentInstance>();
		
		// Get the attributes we need, the vertices (position data) is used to perform a world space triangle intersection
		// The uv attribute is to compute the uv coordinates when a triangle is hit
		MeshInstance& mesh = mIntersectMesh->getMeshInstance();
		VertexAttribute<glm::vec3>& vertices = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		VertexAttribute<glm::vec3>& uvs = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));

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
				mMouseUvPosition = utility::interpolateVertexAttr<glm::vec3>(uv_triangle_data, bary_coord);
				break;
			}
		}
	}
}