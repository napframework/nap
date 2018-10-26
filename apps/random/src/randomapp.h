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
#include <utility/datetimeutils.h>
#include <scene.h>
#include <orthocameracomponent.h>

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

		// Resources
		rtti::ObjectPtr<RenderWindow>						mRenderWindow = nullptr;
		rtti::ObjectPtr<Scene>								mScene = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSceneCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mVideo = nullptr;
		rtti::ObjectPtr<EntityInstance>						mClouds = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSun = nullptr;
		rtti::ObjectPtr<EntityInstance>						mCombination = nullptr;
		rtti::ObjectPtr<EntityInstance>						mLightRig = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrthoCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbit = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitPath = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitStart = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitEnd = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrbitSun = nullptr;
		rtti::ObjectPtr<RenderTarget>						mCloudRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mSunRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mVideoRenderTarget = nullptr;
		rtti::ObjectPtr<RenderTarget>						mCombineRenderTarget = nullptr;

		// Gui related functionality
		std::unique_ptr<RandomGui>	mGui = nullptr;

		// Store window properties
		glm::ivec2 windowSize;

		/**
		 * Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);

		/**
		 *	Renders the video into it's back-buffer
		 */
		void renderVideo(OrthoCameraComponentInstance& orthoCamera);

		/**
		 *	Renders the combination of the video and clouds into it's own back-buffer
		 */
		void renderCombination(OrthoCameraComponentInstance& orthoCamera);

		/**
		 *	Renders the clouds into it's back-buffer
		 */
		void renderClouds(OrthoCameraComponentInstance& orthoCamera);

		/**
		*	Renders the sun into it's back-buffer
		*/
		void renderSun(OrthoCameraComponentInstance& orthoCamera);
	};                                                                               
}