#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <app.h>
#include <spheremesh.h>

namespace nap
{
	using namespace rtti;

	/**
	 * This demonstrates the use of nap function curves as an animation driver
	 *
	 */
	class AnimationApp : public App
	{
		RTTI_ENABLE(App)
	public:
		AnimationApp(nap::Core& core) : App(core)	{ }

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
		ObjectPtr<EntityInstance> mLineEntity = nullptr;				//< Pointer to the entity that holds the sphere
		ObjectPtr<EntityInstance> mLaserEntity = nullptr;				//< Pointer to the entity that represents the laser canvas
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color

		// Colors
		RGBColorFloat mColorTwo = { 0.784f, 0.411f, 0.411f };			//< Line first color	
		RGBColorFloat mColorOne = { 1.0f, 1.0f, 1.0f };					//< Line second color
		float mBlendSpeed = 1.0f;										//< Line blend speed
		float mLineSize = 0.5f;											//< Size of the line (normalized)
		glm::vec2 mLinePosition = { 0.5f, 0.5f };						//< Position of the line relative to canvas
	};
}