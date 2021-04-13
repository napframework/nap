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
#include <imguiservice.h>
#include <renderservice.h>
#include <app.h>
#include <spheremesh.h>
#include <tweenhandle.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class TweenService;

	using namespace rtti;

	/**
	 * TweenApp demonstrates the use of the mod_naptween module
	 * It moves the sphere along a plane using tweens, the user can change properties of the tween and select a new target for the sphere by clicking on the plane
	 */
	class TweenApp : public App
	{
	RTTI_ENABLE(App)
	public:
		TweenApp(nap::Core& core) : App(core)	{ }

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
		/**
		 * Used to find world coordinates of pointer press on plane and call createTween
		 * @param event the mouse pointer event
		 */
		void doTrace(const PointerEvent& event);

		/**
		 * creates tween
		 * @param pos world position of target of tween
		 */
		void createTween(const glm::vec3& pos);

		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		TweenService* mTweenService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;				//< Pointer to the entity that holds the camera
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		ObjectPtr<EntityInstance> mSphereEntity = nullptr;				//< Pointer to the bouncing ball entity
		ObjectPtr<EntityInstance> mPlaneEntity = nullptr;				//< Pointer to the plane entity

		// Tween properties
		float mTweenDuration = 1.0f;									//< Tween duration
		ETweenEaseType mCurrentTweenType = ETweenEaseType::CUBIC_OUT;		//< Tween ease type
		ETweenMode mCurrentTweenMode = ETweenMode::NORMAL;				//< Tween mode
		std::unique_ptr<TweenHandle<glm::vec3>> mMovementTweenHandle; 	//< Handle of tween of sphere movement
		std::unique_ptr<TweenHandle<float>> 	mAnimationTweenHandle; 	//< Handle of animation tween of plane shader
		float mAnimationIntensity = 0.0f;								//< Handle of animation intensity, used in plane shader
		glm::vec2 mAnimationPos = { 0.0f, 0.0f };				//< Animation position in UV space, used in plane shader
	};
}