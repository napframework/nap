/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Core includes
#include <nap/resourcemanager.h>
#include <nap/resourceptr.h>

// Module includes
#include <renderservice.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderadvancedservice.h>
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <imguiservice.h>
#include <color.h>

// Local includes
#include "../applet.h"
#include "meshpreviewappletgui.h"

namespace napkin
{
	using namespace nap;
	using namespace nap::rtti;

	/**
	 * Simple mesh preview application.
	 * Allows users to load and visualize all mesh (nap::IMesh) types.
	 */
	class MeshPreviewApplet : public napkin::Applet
	{
		friend class MeshPreviewAppletGUI;
		RTTI_ENABLE(napkin::Applet)
	public:
		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		MeshPreviewApplet(nap::Core& core);
		
		/**
		 * Initialize all the services and app specific data structures
		 * @param error contains the error code when initialization fails
		 * @return if initialization succeeded
		*/
		bool init(utility::ErrorState& error) override;
		
		/**
		 * Update is called every frame, before render.
		 * @param deltaTime the time in seconds between calls
		 */
		void update(double deltaTime) override;

		/**
		 * Render is called after update. Use this call to render objects to a specific target
		 */
		void render() override;

		/**
		 * Called when the app receives a window message.
		 * @param windowEvent the window message that occurred
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 * Called when the app receives an input message (from a mouse, keyboard etc.)
		 * @param inputEvent the input event that occurred
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;

	private:
		ResourceManager*			mResourceManager = nullptr;			///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;			///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;			///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;				//< Manages gui related update / draw calls
		ObjectPtr<Scene>			mScene = nullptr;					///< Pointer to the main scene
		RenderAdvancedService*		mRenderAdvancedService = nullptr;	///< Render advanced service

		ObjectPtr<EntityInstance>	mLoaderEntity = nullptr;			//< Pointer to the render entity
		ObjectPtr<EntityInstance>	mTextEntity = nullptr;				//< Pointer to the entity that can display text
		ObjectPtr<EntityInstance>	mPerspectiveCamEntity = nullptr;	//< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mOrthographicCamEntity = nullptr;	//< Pointer to the entity with an orthographic camera
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			//< Pointer to the render window
		ObjectPtr<EntityInstance>	mLightsEntity = nullptr;			//< Pointer to the light entity

		RGBAColorFloat mClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };		//< Current clear color
		std::unique_ptr<MeshPreviewAppletGUI> mGUI;						//< Applet gui helper class
	};
}

