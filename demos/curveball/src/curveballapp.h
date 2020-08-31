#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <app.h>
#include <spheremesh.h>	

namespace nap
{
	using namespace rtti;

	/**
	 * This demo demonstrates how to use a Curve (resource) to animate a sphere.
	 * Use Napkin (our editor) to update or alter the various points of the curve.
	 * Edits to the curve in Napkin are directly applied to the animation of the sphere in the live running demo!
	 * Curve edits are stored in the curveball.json file and applied immediately when the file is saved.
	 * The component inside mod_curveball actually animates the sphere based on the value read from the curve.
	 */
	class CurveballApp : public App
	{
		RTTI_ENABLE(App)
	public:
		CurveballApp(nap::Core& core) : App(core)	{ }

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
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;				//< Pointer to the entity that holds the camera
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		ObjectPtr<EntityInstance> mSphereEntity = nullptr;				//< Pointer to the bouncing ball entity
		ObjectPtr<EntityInstance> mPlaneEntity = nullptr;				//< Pointer to the plane entity
	};
}