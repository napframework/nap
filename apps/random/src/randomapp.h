#pragma once

// Local Includes
#include "randomgui.h"

// External Includes
#include <app.h>
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <nap/logger.h>
#include <rendertarget.h>
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <nap/windowresource.h>
#include <inputrouter.h>
#include <entity.h>
#include <artnetcontroller.h>
#include <nap/datetime.h>
#include <scene.h>
#include <orthocameracomponent.h>
#include <rendertexture2d.h>
#include <controlgroups.h>

namespace nap
{
	class RandomApp : public App
	{
		RTTI_ENABLE(App)
		friend class RandomGui;
	public:
		RandomApp(Core& core) : App(core)	{ }

		/**
		 *	Initialize the random app and it's resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update the random app's resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Render stuff to screen
		 */
		virtual void render() override;

		/**
		 *	Shut down the application
		 */
		virtual int shutdown() override;

		/**
		 *	Register input events
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;

		/**
		 *	Register window events
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;

	private:
		// Nap Objects
		nap::RenderService*									mRenderService = nullptr;
		nap::ResourceManager*								mResourceManager = nullptr;
		nap::SceneService*									mSceneService = nullptr;
		nap::InputService*									mInputService = nullptr;
		nap::IMGuiService*									mGuiService = nullptr;

		// Rendering Objects
		rtti::ObjectPtr<RenderWindow>						mRenderWindow = nullptr;
		rtti::ObjectPtr<RenderTarget>						mSunRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mVideoRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mStaticRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mPartyRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mSoundRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mCombineRenderTarget = nullptr;

		// Control Groups
		rtti::ObjectPtr<ControlGroups>						mControlGroups = nullptr;

		// Scene Objects
		rtti::ObjectPtr<Scene>								mScene = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSceneCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrthoCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSunClouds = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSunGlare = nullptr;
		rtti::ObjectPtr<EntityInstance>						mVideo = nullptr;
		rtti::ObjectPtr<EntityInstance>						mStatic = nullptr;
		rtti::ObjectPtr<EntityInstance>						mCombination = nullptr;
		rtti::ObjectPtr<EntityInstance>						mLightRig = nullptr;
		rtti::ObjectPtr<EntityInstance>						mController = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbit = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitPath = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitStart = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitEnd = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitSun = nullptr;
		rtti::ObjectPtr<EntityInstance>						mParty = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSound = nullptr;

		// Random GUI
		std::unique_ptr<RandomGui>							mGui = nullptr;

		// Store window properties
		glm::ivec2 windowSize;

		/**
		* Called when a window event is received
		*/
		void handleWindowEvent(const WindowEvent& windowEvent);

		/**
		*	Renders the sun into it's back-buffer
		*/
		void renderSun(OrthoCameraComponentInstance& orthoCamera);

		/**
		 *	Renders the video into it's back-buffer
		 */
		void renderVideo(OrthoCameraComponentInstance& orthoCamera);

		/**
		*	Renders the static into it's back-buffer
		*/
		void renderStatic(OrthoCameraComponentInstance& orthoCamera);

		/**
		*	Renders the party into it's back-buffer
		*/
		void renderParty(OrthoCameraComponentInstance& orthoCamera);

		/**
		*	Renders the sound into it's back-buffer
		*/
		void renderSound(OrthoCameraComponentInstance& orthoCamera);

		/**
		 *	Renders the combination into it's back-buffer
		 */
		void renderCombination(OrthoCameraComponentInstance& orthoCamera);
	};                                                                               
}