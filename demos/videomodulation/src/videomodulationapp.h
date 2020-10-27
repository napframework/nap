/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <videoservice.h>
#include <renderservice.h>
#include <video.h>
#include <app.h>
#include <imguiservice.h>
#include <color.h>
#include <smoothdamp.h>
#include <videoplayer.h>

namespace nap
{
	using namespace rtti;

	/**
	* Demo application that is called from within the main loop
	*
	* Plays a set of videos. You can select / add or remove videos in JSON
	* The video texture is applied to the background plane and a selection of custom FBX meshes
	* The vertices of the mesh are displaced based on the current video grey scale value.
	* Use the left mouse + mouse button to tumble the camera, right mouse button to zoom
	*
	* This application uses it's own module: mod_videomodulation. 
	* That module exposes 1 resource (VideoMeshFromFile) and 1 component (SelectVideoMeshComponent).
	* The video mesh from file loads an fbx ans adds 2 attributes that are necessary for displacement. 
	* The SelectVideoMeshComponent changes the mesh to display. 
	* 
	* Video playback is handled using FFMpeg. The VideoPlayer allows you to select a video from the play-list, defined in JSON.
	* To limit CPU usage, the video player does not translate the YUV textures to RGB values, that's best to do on the GPU using a shader.
	* The nap::RenderVideoComponent converts the YUV textures of the video player into a single RGB texture using a
	* build in shader and render target.
	* 
	* The resulting RGB video texture is used as an input to both the background plane and the foreground (displacement) mesh.
	* In the background it's shown using a combination of 2 colors, in the foreground the video data is used to
	* displace and color the mesh vertices. Play around with the on screen controls to select a different mesh or video
	* 
	* After rendering the off screen passes, the background is rendered using an orthographic camera and
	* the displacement mesh is rendered on top of the background using a perspective camera.
	*
	* The displacement mesh uses a basic light setup to calculate specular highlights and diffusion.
	* The light position is hard coded in the displacement shader. The camera position is set
	* every frame. The video texture (in combination with) the vertex colors are used to define
	* the specular intensity per fragment
	* 
	* Mouse and key events are forwarded to the input service, the input service collects input events
	* and processes all of them on update. Because NAP does not have a default space (objects can
	* be rendered in multiple ways), you need to specify what input actually means to the application.
	* The input router does that for you. This demo uses the default one that forwards the events to every input component
	* Refer to the cpp-update() call for more information on handling input
	*/
	class VideoModulationApp : public App
	{
		RTTI_ENABLE(App)
	public:
		VideoModulationApp(nap::Core& core) : App(core)	{}
		
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
		RenderService*		mRenderService = nullptr;					//< Render Service that handles render calls
		ResourceManager*	mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService*		mSceneService = nullptr;					//< Manages all the objects in the scene
		InputService*		mInputService = nullptr;					//< Input service for processing input
		VideoService*		mVideoService = nullptr;					//< Service for video playback
		IMGuiService*		mGuiService = nullptr;						//< Service used for updating / drawing guis
		
		ObjectPtr<EntityInstance> mOrthoCameraEntity = nullptr;			//< The entity that holds the camera
		ObjectPtr<EntityInstance> mVideoEntity = nullptr;				//< Used to render the video in to a render target
		ObjectPtr<EntityInstance> mBackgroundEntity = nullptr;			//< Renders the video render target to screen in the background
		ObjectPtr<EntityInstance> mDisplacementEntity = nullptr;		//< The entity that holds the displacement mesh
		ObjectPtr<EntityInstance> mPerspCameraEntity = nullptr;			//< Perspective camera

		// video render target
		ObjectPtr<RenderWindow> mRenderWindow = nullptr;				//< Render window
		ObjectPtr<VideoPlayer> mVideoPlayer = nullptr;					//< Video player

		// Background colors
		RGBColorFloat mBackgroundColorOne =	{ 0.066f, 0.078f, 0.149f };	//< Color of the valley
		RGBColorFloat mBackgroundColorTwo = { 0.784f, 0.411f, 0.411f };	//< Color of the peak
		RGBColorFloat mClearColor =			{ 0.000f, 0.000f, 0.000f };	//< Color used for clearing render window
		
		// Selection
		int mCurrentMesh  = 0;
		int mCurrentVideo = 0;

		// Displacement
		float mDisplacement = 0.22f;									//< Total amount of displacement
		float mRandomness = 0.25f;										//< Total amount of displacement deviation

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
