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
#include <scene.h>
#include <renderwindow.h>
#include <entity.h>
#include <imguiservice.h>

// Local includes
#include "../applet.h"

namespace napkin
{
	using namespace nap;
	using namespace nap::rtti;

	/**
	 * Main application that is called from within the main loop
	 */
	class TexturePreviewApplet : public napkin::Applet
	{
		RTTI_ENABLE(napkin::Applet)
	public:
		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		TexturePreviewApplet(Core& core) : napkin::Applet(core) { }
		
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
		ResourceManager*  mResourceManager = nullptr;					//< Manages all the loaded data
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls

		// Resources
		ObjectPtr<Scene> mScene = nullptr;								//< Pointer to the main scene
		ObjectPtr<RenderWindow> mRenderWindow = nullptr;				//< Pointer to the render window;

		// Entities
		ObjectPtr<EntityInstance> mTextEntity = nullptr;				//< Pointer to the entity that can display text
		ObjectPtr<EntityInstance> mAPIEntity = nullptr;					//< Pointer to the api entity

		// 2DTexture
		ObjectPtr<EntityInstance> m2DTextureEntity = nullptr;			//< Pointer to the texture entity
		ObjectPtr<EntityInstance> m2DOrthoCameraEntity = nullptr;		//< Pointer to the ortho camera

		// Cubemap
		ObjectPtr<EntityInstance> mCubeTextureEntity = nullptr;			//< Pointer to the cube texture entity
		ObjectPtr<EntityInstance> mCubePerspCameraEntity = nullptr;		//< Pointer to the cube persp camera entity

		//std::unique_ptr<nap::Texture2D> mActiveTexture = nullptr;		//< Current active texture
		RGBAColorFloat mClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };		//< Current clear color

		void texDetail(std::string&& label, const std::string& value, std::string&& appendix = "");
		void texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument);
	};
}

