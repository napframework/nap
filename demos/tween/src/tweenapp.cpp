/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "tweenapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <mathutils.h>
#include <meshutils.h>
#include <uniforminstance.h>
#include <tweenservice.h>
#include <tweenhandle.h>
#include <tween.h>
#include <renderglobals.h>
#include <imguiutils.h>

// Register this application with RTTI, this is required by the AppRunner to
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TweenApp)
		RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool TweenApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
		mTweenService	= getCore().getService<nap::TweenService>();

		// Get resource manager
		mResourceManager = getCore().getResourceManager();

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");

		// Extract the only scene
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Find the entities we're interested in
		mCameraEntity = scene->findEntity("Camera");
		mSphereEntity = scene->findEntity("Sphere");
		mPlaneEntity = scene->findEntity("Plane");

		mGuiService->selectWindow(mRenderWindow);

		return true;
	}


	/**
	* Forward all the received input messages to the camera input components.
	* The input router is used to filter the input events and to forward them
	* to the input components of a set of entities, in this case our camera.
	* After that we setup the gui.
	*/
	void TweenApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mCameraEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Find the camera location uniform in the material of the sphere
		nap::RenderableMeshComponentInstance& sphere_mesh = mSphereEntity->getComponent<nap::RenderableMeshComponentInstance>();
		auto* ubo =  sphere_mesh.getMaterialInstance().getOrCreateUniform("UBO");
		auto* cam_loc_uniform = ubo->getOrCreateUniform<UniformVec3Instance>("inCameraPosition");

		// Set it to the current camera location
		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
		cam_loc_uniform->setValue(global_pos);

		// Set color
		auto* ball_color = ubo->getOrCreateUniform<UniformVec3Instance>("ballColor");
		ball_color->setValue(mGuiService->getColors().mHighlightColor1.convert<RGBColorFloat>());

		// Find the animation uniform in the material of the plane.
		nap::RenderableMeshComponentInstance& plane_mesh = mPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>();
		ubo = plane_mesh.getMaterialInstance().getOrCreateUniform("UBO");

		/// Set intensity in shader
		auto* animator_intensity_uniform = ubo->getOrCreateUniform<nap::UniformFloatInstance>("animationValue");
		animator_intensity_uniform->setValue(mAnimationIntensity);

		// Set animation position in shader
		auto* animator_pos_uniform = ubo->getOrCreateUniform<nap::UniformVec2Instance>("animationPos");
		animator_pos_uniform->setValue(mAnimationPos);

		// Set plane colors
		auto* color_one = ubo->getOrCreateUniform<nap::UniformVec3Instance>("colorOne");
		auto* color_two = ubo->getOrCreateUniform<nap::UniformVec3Instance>("colorTwo");
		color_one->setValue(mGuiService->getColors().mFront4Color.convert<RGBColorFloat>());
		color_two->setValue(mGuiService->getColors().mDarkColor.convert<RGBColorFloat>());

		// draw the GUI
		if( ImGui::Begin("Tween") )
		{
			// draw some instructions and meta data
			ImGui::Text(getCurrentDateTime().toString().c_str());
			RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
			ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
			ImGui::TextColored(mGuiService->getColors().mHighlightColor2,
				"Click somewhere in the window to move the sphere to that location");

			// change duration of the tween
			if( ImGui::InputFloat("Duration", &mTweenDuration) )
			{
				mTweenDuration = math::max<float>(0.0f, mTweenDuration);

				if( mMovementTweenHandle != nullptr )
				{
					mMovementTweenHandle->getTween().setDuration(mTweenDuration);
				}
			}


			// change tween ease type combo box
			int ease_type = (int)mCurrentTweenType;
			if (ImGui::Combo("Ease", &ease_type, RTTI_OF(nap::ETweenEaseType)))
			{
				mCurrentTweenType = (ETweenEaseType)ease_type;
				if (mMovementTweenHandle != nullptr)
				{
					mMovementTweenHandle->getTween().setEase(mCurrentTweenType);
				}
			}

			// change tween mode combo box
			int tween_mode = (int)mCurrentTweenMode;
			if( ImGui::Combo("Mode", &tween_mode, RTTI_OF(nap::ETweenMode)))
			{
				mCurrentTweenMode = (ETweenMode) tween_mode;
				if( mMovementTweenHandle != nullptr )
				{
					mMovementTweenHandle->getTween().setMode(mCurrentTweenMode);
				}
			}
		}
		ImGui::End();
	}


	void TweenApp::createTween(const glm::vec3& pos)
	{
		// get the sphere transformation
		auto& sphere_transform = mSphereEntity->getComponent<TransformComponentInstance>();

		// get the current sphere position in world coordinates
		glm::vec3 sphere_position = math::extractPosition(sphere_transform.getGlobalTransform());

		// create a tween and store the handle
		mMovementTweenHandle = mTweenService->createTween<glm::vec3>(sphere_position, pos, mTweenDuration, (ETweenEaseType)mCurrentTweenType, (ETweenMode)mCurrentTweenMode);

		// get reference to tween from tween handle
		Tween<glm::vec3>& movement_tween = mMovementTweenHandle->getTween();

		// connect to update signal
		movement_tween.UpdateSignal.connect([this](const glm::vec3& value) {
		  auto& sphere_transform = mSphereEntity->getComponent<TransformComponentInstance>();
		  sphere_transform.setTranslate(value);
		});

		// animate the animation intensity uniform of the plane
		mAnimationIntensity 	= 0.0f;
		mAnimationTweenHandle	= mTweenService->createTween<float>(0.0f, 1.0f, 0.5f, ETweenEaseType::CIRC_OUT);
		mAnimationTweenHandle->getTween().UpdateSignal.connect([this](const float& value)
		{
			mAnimationIntensity = value;
		});
	}

	/**
	 * Render loop is rather straight forward.
	 * All the objects in the scene are rendered at once including the sphere and plane.
	 * This demo doesn't require special render steps.
	 */
	void TweenApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		// This prepares a command buffer and starts a render pass
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Start render pass
			mRenderWindow->beginRendering();

			// Render all objects in the scene at once
			// This includes the line + normals and the laser canvas
			mRenderService->renderObjects(*mRenderWindow, mCameraEntity->getComponent<PerspCameraComponentInstance>());

			// Draw gui to screen
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// Stop recording commands
			mRenderService->endRecording();
		}

		mRenderService->endFrame();
	}


	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window.
	 * On the next update the render service automatically processes all window events.
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void TweenApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void TweenApp::inputMessageReceived(InputEventPtr inputEvent)
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

		if(inputEvent->get_type().is_derived_from<PointerPressEvent>())
		{
			PointerPressEvent* pointer_event = static_cast<nap::PointerPressEvent*>(inputEvent.get());
			doTrace(*pointer_event);
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	void TweenApp::doTrace(const PointerEvent& event)
	{
		// Get the camera and camera transform
		PerspCameraComponentInstance& camera = mCameraEntity->getComponent<PerspCameraComponentInstance>();
		TransformComponentInstance& camera_xform = mCameraEntity->getComponent<TransformComponentInstance>();

		// Get screen (window) location from mouse event
		glm::ivec2 screen_loc = { event.mX, event.mY };

		// Get object to world transformation matrix
		TransformComponentInstance& world_xform = mPlaneEntity->getComponent<TransformComponentInstance>();

		// Get Mesh Instance
		MeshInstance& mesh = mPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMeshInstance();

		// Get the attributes we need, the vertices (position data) is used to perform a world space triangle intersection
		// The uv attribute is to compute the uv coordinates when a triangle is hit
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
		while (!triangle_it.isDone())
		{
			// Use the indices to get the vertex positions
			Triangle triangle = triangle_it.next();

			tri_vertices[0] = (math::objectToWorld(vertices[triangle[0]], world_xform.getGlobalTransform()));
			tri_vertices[1] = (math::objectToWorld(vertices[triangle[1]], world_xform.getGlobalTransform()));
			tri_vertices[2] = (math::objectToWorld(vertices[triangle[2]], world_xform.getGlobalTransform()));

			// find bary centric coordinates
			glm::vec3 bary_coord;
			if (utility::intersect(cam_pos, screen_to_world_ray, tri_vertices, bary_coord))
			{
				TriangleData<glm::vec3> uv_triangle_data = triangle.getVertexData(uvs);
				mAnimationPos = utility::interpolateVertexAttr<glm::vec3>(uv_triangle_data, bary_coord);

				// use bary centric coordinates to find world position
				glm::vec3 world_pos = (tri_vertices[0] * (1.0f - bary_coord.x - bary_coord.y)) + (tri_vertices[1] * bary_coord.x) + (tri_vertices[2] * bary_coord.y);

				// create a new tween to world position
				createTween(world_pos);

				break;
			}
		}
	}

	int TweenApp::shutdown()
	{
		return 0;
	}
}
