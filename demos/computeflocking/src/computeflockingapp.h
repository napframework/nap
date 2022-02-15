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

namespace nap
{
	// Forward declares
	class FlockingSystemComponentInstance;

	/**
	* Demo application that is called from within the main loop
	*
	* Shows upward floating textured particles
	* Use the 'wasd' keys and the left mouse button to move through the scene
	*
	* This application uses it's own module: mod_dynamicgeo. In there sits an object
	* that creates, removes and updates the particles. It also renders the particles as a single mesh to screen
	* It demonstrates one important thing: the creation of dynamic geometry. Because the particle
	* count changes constantly the mesh is updated every frame to reflect those changes. 
	* Refer to particleemittercomponent.h for more information
	*
	* Mouse and key events are forwarded to the input service, the input service collects input events
	* and processes all of them on update. Because NAP does not have a default space (objects can
	* be rendered in multiple ways), you need to specify what input actually means to the application.
	* The input router does that for you. This demo uses the default one that forwards the events to every input component
	* Refer to the cpp-update() call for more information on handling input
	*
	* We simply render all the objects in the scene to the primary screen at once. 
	* This makes sense because there is only 1 drawable object (the particle simulation) and
	* we don't use any other render targets. 
	*
	* The particle object is an example and not something that should be considered final.
	* It demonstrates how you can modify a buffer and use that buffer to create a mesh that is drawn to screen
	* More information about rendering, scenes etc. can be found in the other, more basic, examples.
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

		/**
		 * Collects required resources
		 */
		bool reload(utility::ErrorState& errorState);

		/**
		 * Caches required resources
		 */
		void cache();

	private:
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< IMGui service

		rtti::ObjectPtr<RenderWindow> mRenderWindow;					//< Pointers to the render window
		rtti::ObjectPtr<EntityInstance> mDefaultInputRouter;			//< Routes input events to the input component
		rtti::ObjectPtr<EntityInstance> mCameraEntity;					//< Entity that holds the camera
		rtti::ObjectPtr<EntityInstance> mOrthoCameraEntity;				//< Entity that holds the ortho camera

		rtti::ObjectPtr<EntityInstance> mFlockingSystemEntity;
		rtti::ObjectPtr<EntityInstance> mRenderEntity;
		rtti::ObjectPtr<EntityInstance> mWorldEntity;
		rtti::ObjectPtr<EntityInstance> mBoundsEntity;

		rtti::ObjectPtr<RenderTarget> mRenderTarget;

		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		std::unique_ptr<ParameterGUI> mParameterGUI;

		int mNumBoids;

		// RenderContrast uniforms
		ParameterFloat* mContrastParam = nullptr;
		UniformFloatInstance* mContrastUniform = nullptr;

		ParameterFloat* mBrightnessParam = nullptr;
		UniformFloatInstance* mBrightnessUniform = nullptr;

		ParameterFloat* mSaturationParam = nullptr;
		UniformFloatInstance* mSaturationUniform = nullptr;

		// RenderComposite uniforms
		ParameterFloat* mBlendParam = nullptr;
		UniformFloatInstance* mBlendUniform = nullptr;

		// Bounds radius
		ParameterFloat* mBoundsRadiusParam = nullptr;

		std::string mSelectedPreset;
		nap::Slot<> mCacheSlot = { [&]() -> void { cache(); } };
		nap::Slot<> mReloadSlot = { [&]() -> void { utility::ErrorState error_state; reload(error_state); } };

		// RenderComponents
		FlockingSystemComponentInstance*	mFlockingSystemComponent = nullptr;
		RenderBloomComponentInstance*		mBloomComponent = nullptr;
		RenderToTextureComponentInstance*	mContrastComponent = nullptr;
		RenderToTextureComponentInstance*	mCompositeComponent = nullptr;
		RenderableMeshComponentInstance*	mTargetPointMeshComponent = nullptr;
		RenderableMeshComponentInstance*	mBoundsMeshComponent = nullptr;
	};
}
