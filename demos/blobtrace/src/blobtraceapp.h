/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// External Includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <app.h>
#include <planemesh.h>
#include <smoothdamp.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a plane with a pulsating blob in the middle of the canvas
	 * The blob is procedurally generated in a shader, including lighting
	 * Move the mouse to change the location of the blob on the canvas
	 * Use the left mouse button to rotate the camera
	 * Use the right mouse button to zoom
	 * 
	 * This demo uses ray-casting to resolve the position of the blob on the plane
	 * A ray is cast from the screen in to the world when the mouse is moved
	 * When the ray hits a triangle of the intersection plane the returned (barycentric) coordinates are used to compute the uv coordinates at the hit location
	 * All movement and positioning of the blob happens in uv space!
	 * The intersection plane only has 2 triangles. The plane that is rendered to screen has more to create the displacement effect by the blob.vert shader
	 * All shading and lighting is handled by the blob.fragment shader, including the generation of the normals
	 * 
	 * On update a set of shader variables is set, most importantly the accumulated time. 
	 * The blob slowly moves towards it's target. The SmoothOperator is responsible for this effect
	 * Every frame that operator is fed with the current mouse location and moves the blob target towards it using a cheap dampening model.
	 * The velocity of that movement is used by the blob shader to change the blob size and frequency.
	 *
	 * Except for the ray-casting this demo is rather simple. 
	 * The only components are a camera and two planes: one plane is used for intersection the other for rendering
	 * 
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
	 */
	class BlobTraceApp : public App
	{
		RTTI_ENABLE(App)
	public:
		BlobTraceApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;									//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;								//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;										//< Manages all the objects in the scene
		InputService* mInputService = nullptr;										//< Input service for processing input
		IMGuiService* mGuiService = nullptr;										//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;										//< Pointer to the render window		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;							//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance> mPlaneEntity = nullptr;							//< Pointer to the entity that holds the sphere
		ObjectPtr<PlaneMesh> mIntersectMesh = nullptr;								//< Plane that is used for ray intersection
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };				//< GUI text highlight color
		bool mMouseDown = false;													//< If the mouse button is currently pressed
		float mTime = 0.0f;															//< Current application running time
		glm::vec3 mMouseUvPosition = { 0.5,0.5,0.0 };								//< Current mouse position in uv space
		math::Vec3SmoothOperator mUVSmoother = { glm::vec3(0.5,0.5,0.0), 1.0f };	//< Blends the target to the current mouse position
			
		/**
		 * Shoots a ray from the camera in screen space in to the world
		 * When intersected with the plane it calculates the plane's uv intersection location
		 * This location is stored and used as a target for the blob when rendering
		 * @param event the mouse event that contains the current mouse location
		 */
		void doTrace(const PointerEvent& event);
	};
}
