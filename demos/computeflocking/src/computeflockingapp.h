/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderbloomcomponent.h>
#include <rendertotexturecomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <rendertarget.h>
#include <app.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <parametergui.h>
#include <parameternumeric.h>
#include <nap/signalslot.h>
#include <parametersimple.h>
#include <rendergnomoncomponent.h>

namespace nap
{
	// Forward declares
	class FlockingSystemComponentInstance;
	class PerspCameraComponentInstance;
	class BoidTargetTranslateComponentInstance;

	/**
	 * Demo application that demonstrates the use of compute to update and render a 3D flocking system.
	 *
	 * This application depends on its corresponding module: mod_computeflocking. This includes the object
	 * nap::FlockingSystemComponent, which manages a flocking system that can be rendered as a single mesh.
	 *
	 * This demo is somewhat similar to `computeparticles`, but far more complex on the GPU side. We therefore
	 * recommend studying this demo before moving on to this one.
	 * 
	 * This demo includes a compute shader in which each thread reads from a storage buffer thousands of times, and
	 * leverages shared memory to do so faster. The world is also rendered offscreen, after which a stack of
	 * post-processing effects applied to the color texture. The final texture is then applied to a quad and rendered
	 * to a window with an orthographic camera.
	 * 
	 * The compute shader `flock.comp` generates a nap::StructGPUBuffer comprising of boid data. The most important
	 * properties of a boid are its position, velocity (direction and magnitude) and orientation (a quaternion). The
	 * layout and contents of the boid buffers described in JSON match those defined in `flock.comp`.
	 *
	 * The application scene graph includes an instance of a flocking system that we want to render. The component is
	 * designed such that, in order to update the boids, we must call FlockingSystemComponent::compute(). This call
	 * pushes a compute shader dispatch command to the current command buffer. As all compute work must be recorded to
	 * the compute command buffer, compute work is always dispatched inside App::render(), between
	 * RenderService::beginComputeRecording() and RenderService::endComputeRecording().
	 */
	class ComputeFlockingApp : public App
	{
		RTTI_ENABLE(App)
	public:
		ComputeFlockingApp(nap::Core& core) : App(core) { }

		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all render-able objects to the GPU
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
		RenderService* mRenderService = nullptr;							//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;						//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;								//< Manages all the objects in the scene
		InputService* mInputService = nullptr;								//< Input service for processing input
		IMGuiService* mGuiService = nullptr;								//< IMGui service

		rtti::ObjectPtr<RenderWindow> mRenderWindow = nullptr;				//< Pointers to the render window
		rtti::ObjectPtr<EntityInstance> mDefaultInputRouter = nullptr;		//< Routes input events to the input component
		rtti::ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< Entity that holds the camera
		rtti::ObjectPtr<EntityInstance> mOrthoCameraEntity = nullptr;		//< Entity that holds the ortho camera

		rtti::ObjectPtr<EntityInstance> mFlockingSystemEntity = nullptr;	//< Holds the flocking system abd required components
		rtti::ObjectPtr<EntityInstance> mRenderEntity = nullptr;			//< Holds rendering operations as components
		rtti::ObjectPtr<EntityInstance> mWorldEntity = nullptr;				//< Holds components to render
		rtti::ObjectPtr<EntityInstance> mBoundsEntity = nullptr;			//< Holds world bounds
		rtti::ObjectPtr<EntityInstance> mTargetEntity = nullptr;			//< Holds boid target
		rtti::ObjectPtr<RenderTarget> mRenderTarget = nullptr;				//< Offscreen render target
		rtti::ObjectPtr<ParameterGUI> mParameterGUI = nullptr;				//< Parameter GUI

		// Parameters
		ResourcePtr<Parameter> mContrastParam;
		ResourcePtr<Parameter> mBrightnessParam;
		ResourcePtr<Parameter> mSaturationParam;

		ResourcePtr<ParameterFloat> mBoundsRadiusParam;
		ResourcePtr<Parameter> mShowBoundsParam;
		ResourcePtr<Parameter> mBlendParam;

		bool mShowGUI = true;
	};
}
