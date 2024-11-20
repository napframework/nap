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
#include <imagefromfile.h>
#include <imguiservice.h>
#include <apievent.h>

// Local includes
#include "../applet.h"

namespace nap
{
	using namespace rtti;

	/**
	 * Main application that is called from within the main loop
	 */
	class TexturePreviewApplet : public napkin::Applet
	{
		RTTI_ENABLE(napkin::Applet)
	public:

		// Signature callback names and arguments
		static constexpr const char* loadCmd1 = "LoadTexture";
		static constexpr const char* loadArg1 = "data";

		/**
		 * Constructor
		 * @param core instance of the NAP core system
		 */
		TexturePreviewApplet(nap::Core& core) : napkin::Applet(core) { }
		
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
		
		/**
		 * Called when the app is shutting down after quit() has been invoked
		 * @return the application exit code, this is returned when the main loop is exited
		 */
		virtual int shutdown() override;

	private:
		ResourceManager*			mResourceManager = nullptr;			///< Manages all the loaded data
		RenderService*				mRenderService = nullptr;			///< Render Service that handles render calls
		SceneService*				mSceneService = nullptr;			///< Manages all the objects in the scene
		InputService*				mInputService = nullptr;			///< Input service for processing input
		IMGuiService*				mGuiService = nullptr;				///< Manages gui related update / draw calls
		ObjectPtr<Scene>			mScene = nullptr;					///< Pointer to the main scene

		ObjectPtr<EntityInstance>	mTextEntity = nullptr;				//< Pointer to the entity that can display text
		ObjectPtr<EntityInstance>	mAPIEntity = nullptr;				//< Pointer to the api entity
		ObjectPtr<EntityInstance>	mTextureEntity = nullptr;			//< Pointer to the texture entity
		ObjectPtr<EntityInstance>	mOrthoEntity = nullptr;				//< Pointer to the ortho camera
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			//< Pointer to the render window

		ObjectPtr<APISignature>		mLoadSignature = nullptr;			//< Pointer to the api text signature
		void onLoadRequested(const nap::APIEvent& apiEvent);			//< Loads a texture from JSON
		nap::Slot<const nap::APIEvent&> mLoadRequestedSlot = { this, &TexturePreviewApplet::onLoadRequested };

		std::unique_ptr<nap::Texture2D> mActiveTexture = nullptr;
	};
}
