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
#include <imguiservice.h>
#include <renderservice.h>
#include <cvservice.h>
#include <app.h>
#include <cvcapturedevice.h>
#include <rendertexture2d.h>

namespace nap
{
	using namespace rtti;

	/**
	 * OpenCV Demo application that is called from within the main loop
	 *
	 * Shows a set of detected (classified) objects in 3D and 2D.
	 * Objects are detected using an OpenCV HaarCascade classifier.
	 * The classified objects are rendered to texture and the viewport.
	 *
	 * This demo makes use of your first (default) Web-cam if present.
	 * Use the on-screen controls to switch detection source (Video / Web-cam).
	 *
	 * Note that this is a rather advanced demo that uses it's own module: mod_facedetection.
	 * To render the detected objects to screen the 'RenderableClassifyComponent' is used.
	 * The 'CaptureToTextureCompoment' is used to upload the captured frames to a GPU texture.
	 * Refer to the documentation of both components to get a better understanding of what they do.
	 *
	 * You can use the left mouse button to orbit around the center and 
	 * the right mouse button to zoom in on the object.
	 */
	class FaceDetectionApp : public App
	{
		RTTI_ENABLE(App)
	public:
		FaceDetectionApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all render-able objects to the GPU
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

		/**
		 * Updates the GUI
		 */
		void updateGUI();

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		CVService* mCVService = nullptr;								//< Pointer to the opencv service

		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		
		ObjectPtr<EntityInstance>	mPerspectiveCamEntity = nullptr;	//< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mOrthographicCamEntity = nullptr;	//< Pointer to the entity with an orthographic camera
		ObjectPtr<EntityInstance>	mOpenCVEntity = nullptr;			//< Pointer to the OpenCV entity
		ObjectPtr<EntityInstance>	mTextEntity = nullptr;				//< Pointer to the entity that can draw text

		ObjectPtr<CVCaptureDevice>	mCameraCaptureDevice = nullptr;		//< Pointer to the camera capture device, captures the camera adapter
		ObjectPtr<CVCaptureDevice>	mVideoCaptureDevice = nullptr;		//< Pointer to the video device, captures the video adapter

		ObjectPtr<RenderTexture2D>	mCameraCaptureTexture = nullptr;	//< Pointer to the texture we need to capture
		ObjectPtr<RenderTexture2D>	mVideoCaptureTexture = nullptr;		//< Pointer to the video texture we need to capture
		ObjectPtr<RenderTexture2D>	mCameraOutputTexture = nullptr;		//< Pointer to the camera output texture, which contains the detected blobs
		ObjectPtr<RenderTexture2D>	mVideoOutputTexture = nullptr;		//< Pointer to the video output texture, which contains the detected blobs

		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		int mCurrentVideoFrame = 0;										//< Holds currently selected frame from video stream
		int mCurrentSelection = 0;										//< Holds what to render to render to the viewport (video or camera)
	};
}
