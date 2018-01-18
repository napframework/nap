#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <videoservice.h>
#include <video.h>
#include <app.h>
#include <rendertarget.h>
#include <imguiservice.h>
#include <color.h>

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class VideoApp : public App
	{
		RTTI_ENABLE(App)
	public:
		VideoApp(nap::Core& core) : App(core)	{}
		
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
		 *	Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);

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
		RenderService*		mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager*	mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService*		mSceneService = nullptr;					//< Manages all the objects in the scene
		InputService*		mInputService = nullptr;					//< Input service for processing input
		VideoService*		mVideoService = nullptr;					//< Service for video playback
		IMGuiService*		mGuiService = nullptr;						//< Service used for updating / drawing guis
		
		ObjectPtr<EntityInstance> mOrthoCameraEntity = nullptr;			//< The entity that holds the camera
		ObjectPtr<EntityInstance> mVideoEntity = nullptr;				//< Used to render the video in to a render target
		ObjectPtr<EntityInstance> mBackgroundEntity = nullptr;			//< Renders the video render target to screen in the background
		
		// The video render target
		ObjectPtr<RenderTarget>	mVideoRenderTarget = nullptr;			//< Video render target
		ObjectPtr<RenderWindow> mRenderWindow;							//< Render window
		ObjectPtr<Video> mVideoResource = nullptr;						//< Our video resource

		// Background colors
		RGBColorFloat mBackgroundColorOne =	{ 0.066f, 0.078f, 0.149f };	//< Color of the valley
		RGBColorFloat mBackgroundColorTwo = { 0.784f, 0.411f, 0.411f };	//< Color of the peak
		RGBColorFloat mClearColor =			{ 0.000f, 0.000f, 0.000f };	//< Color used for clearing render window

		/**
		 * Sets up the GUI every frame	
		 */
		void updateGui();

		/**
		* Positions the background plane in the center of the window,
		* based on the window resolution and video size
		*/
		void positionBackground();
	};
}
