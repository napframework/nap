/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <app.h>
#include <imguiservice.h>
#include <renderservice.h>

namespace nap
{
	/**
	 * Demo application that demonstrates the use of compute to update and render lots of particles really fast.
	 *
	 * This application depends on its corresponding module: mod_computeparticles. This includes the object
	 * nap::ParticleVolumeComponent, which manages a group of particles that can be rendered as a single mesh.
	 * 
	 * All particle data is computed on the GPU by means of a compute shader that reads from and writes to storage
	 * buffers containing particle data. In this demo, the particle data is stored in a nap::StructGPUBuffer, the layout
	 * and contents of which are described in JSON. In this case, the particle buffer contains 1 mln elements, each of
	 * which comprises of a position, velocity and rotation. Storage buffers are bound to compute shaders via a
	 * nap::ComputeMaterial. Subsequently, compute shaders can be dispatched using a nap::ComputeComponent, which combines
	 * invocation information with a compute material and handles memory access and execution synchronization.
	 * 
	 * The application scene graph includes an instance of a particle volume that we want to render. The component is
	 * designed such that, in order to update the particles, we must call ParticleVolumeComponent::compute(). This call
	 * pushes a compute shader dispatch command to the current command buffer. As all compute work must be recorded to
	 * the compute command buffer, compute work is always dispatched inside App::render(), between
	 * RenderService::beginComputeRecording() and RenderService::endComputeRecording().
	 */
	class ComputeParticlesApp : public App
	{
		RTTI_ENABLE(App)
	public:
		ComputeParticlesApp(nap::Core& core) : App(core)				{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, dispatches compute work and pushes all renderable objects to the GPU
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
		
	private:
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< IMGui service

		rtti::ObjectPtr<RenderWindow> mRenderWindow;					//< Pointers to the render window
		rtti::ObjectPtr<EntityInstance> mDefaultInputRouter;			//< Routes input events to the input component
		rtti::ObjectPtr<EntityInstance> mCameraEntity;					//< Entity that holds the camera
		rtti::ObjectPtr<EntityInstance> mParticleEntity;				//< Entity that emits the particles
	};
}
