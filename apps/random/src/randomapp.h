#pragma once

// Local Includes
#include "randomgui.h"
#include "randomorbit.h"
#include "randomshaders.h"

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
#include <rendertexture2d.h>

namespace nap
{
	enum class LightingModes { None, Sun, Video, Static };

	class RandomApp : public App
	{
		RTTI_ENABLE(App)
		friend class RandomGui;
		friend class RandomOrbit;
		friend class RandomShaders;
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
		rtti::ObjectPtr<RenderTarget>						mCombineRenderTarget = nullptr;

		// Texture Objects
		rtti::ObjectPtr<RenderTexture2D>					mNoneColorTexture = nullptr;
		rtti::ObjectPtr<RenderTexture2D>					mSunColorTexture = nullptr;
		rtti::ObjectPtr<RenderTexture2D>					mVideoColorTexture = nullptr;

		// Scene Objects
		rtti::ObjectPtr<Scene>								mScene = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSceneCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mOrthoCamera = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSunClouds = nullptr;
		rtti::ObjectPtr<EntityInstance>						mSunGlare = nullptr;
		rtti::ObjectPtr<EntityInstance>						mVideo = nullptr;
		rtti::ObjectPtr<EntityInstance>						mCombination = nullptr;
		rtti::ObjectPtr<EntityInstance>						mLightRig = nullptr;

		// Random App components
		std::unique_ptr<RandomGui>							mGui = nullptr;
		std::unique_ptr<RandomOrbit>						mOrbit = nullptr;
		std::unique_ptr<RandomShaders>						mShaders = nullptr;

		// Store window properties
		glm::ivec2 windowSize;

		// Lighting Modes
		const char*											mLightingModes[4] = { "None", "Sun", "Video", "Static" };
		int													mLightingModeInt = 0;
		LightingModes										mLightingModeEnum = LightingModes::None;
		LightingModes										mOldLightingModeEnum;

		/**
		* Called the lighting mode changed
		*/
		void updateLightingMode();
		void resetOldLightingMode();

		/**
		* Get the appropriate texture for a lighing mode
		*/
		nap::Texture2D& getTextureForLightingMode(LightingModes& lightingMode);

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
		 *	Renders the combination into it's back-buffer
		 */
		void renderCombination(OrthoCameraComponentInstance& orthoCamera);
	};                                                                               
}