/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "facedetectionapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <renderable3dtextcomponent.h>
#include <orthocameracomponent.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include <rendertotexturecomponent.h>
#include <renderableclassifycomponent.h>
#include <cvvideo.h>
#include <cvcamera.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FaceDetectionApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool FaceDetectionApp::init(utility::ErrorState& error)
	{		
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
		mCVService		= getCore().getService<nap::CVService>();
		
		// Limit number of OpenCV cores
		mCVService->setThreadCount(2);

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("facedetection.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
		mCameraCaptureDevice = mResourceManager->findObject<nap::CVCaptureDevice>("CameraCaptureDevice");
		mVideoCaptureDevice = mResourceManager->findObject<nap::CVCaptureDevice>("VideoCaptureDevice");
		mCameraCaptureTexture = mResourceManager->findObject<nap::RenderTexture2D>("CameraCaptureTexture");
		mVideoCaptureTexture = mResourceManager->findObject<nap::RenderTexture2D>("VideoCaptureTexture");
		mCameraOutputTexture = mResourceManager->findObject<nap::RenderTexture2D>("CameraOutputTexture");
		mVideoOutputTexture = mResourceManager->findObject<nap::RenderTexture2D>("VideoOutputTexture");

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Fetch the entities
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");
		mOpenCVEntity = scene->findEntity("OpenCV");
		mTextEntity = scene->findEntity("Text");

		// Select GUI window
		mGuiService->selectWindow(mRenderWindow);

		return true;
	}


	/**
	* Forward all the received input messages to the camera and update the GUI.
	*/
	void FaceDetectionApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;

		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Update GUI
		updateGUI();

		// Get component that contains all the captured blobs.
		// There are 2 capture entities, pick the one currently selected and fetch the capture component
		nap::EntityInstance& capture_entity = (*mOpenCVEntity)[mCurrentSelection];
		nap::EntityInstance& blob_entity = capture_entity[0][0];
		RenderableClassifyComponentInstance& classify_render_comp = blob_entity.getComponent<RenderableClassifyComponentInstance>();

		// Get component that can render 2D text.
		Renderable2DTextComponentInstance& text_comp = mTextEntity->getComponent<Renderable2DTextComponentInstance>();

		// Get number of blobs and resize lines of text based on that count.
		const std::vector<glm::vec4>& locs = classify_render_comp.getLocations();
		text_comp.resize(locs.size());

		// Set text for next draw operation
		utility::ErrorState error;
		for (int i = 0; i < locs.size(); i++)
		{
			text_comp.setText(i, utility::stringFormat("Blob %d", i + 1), error);
		}
	}

	
	/**
	 * First render the camera and video, together with the detected blobs, to texture.
	 * After that render the blobs of the selected source (video / webcam) to screen in 3D.
	 * Text is placed and rendered on top of every detected blob, before the GUI 
	 * is rendered and the buffers are swapped.
	 */
	void FaceDetectionApp::render()
	{
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Render detected blobs to texture for all OpenCV capture entities
		if (mRenderService->beginHeadlessRecording())
		{
			for (auto& entity : mOpenCVEntity->getChildren())
			{
				entity->getComponent<RenderToTextureComponentInstance>().draw();
			}
			mRenderService->endHeadlessRecording();
		}

		// Render everything to screen
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Start render pass
			mRenderWindow->beginRendering();

			// Find the perspective camera
			nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

			// Find objects to render
			std::vector<nap::RenderableComponentInstance*> components_to_render;

			// Render detected blobs + ground plane to viewport for the selected OpenCV capture entity.
			nap::EntityInstance& capture_entity = (*mOpenCVEntity)[mCurrentSelection];
			for (auto& entity : capture_entity[0].getChildren())
			{
				RenderableComponentInstance& render_comp = entity->getComponent<RenderableComponentInstance>();
				components_to_render.emplace_back(&render_comp);
			}
			mRenderService->renderObjects(*mRenderWindow, persp_camera, components_to_render);

			// Get component that can render 2D text.
			Renderable2DTextComponentInstance& text_comp = mTextEntity->getComponent<Renderable2DTextComponentInstance>();

			// Get entity that draws the 3D blobs, it's the first child of the 3D visualize entity.
			nap::EntityInstance& blob_entity = capture_entity[0][0];
			RenderableClassifyComponentInstance& classify_render_comp = blob_entity.getComponent<RenderableClassifyComponentInstance>();

			// Extract world space blob locations and draw text
			const std::vector<glm::vec4>& locs = classify_render_comp.getLocations();

			// Draw text above blob
			utility::ErrorState error;
			for (int i = 0; i < locs.size(); i++)
			{
				// Get blob location in 3D
				glm::vec3 blob_pos = locs[i];
				blob_pos.y += locs[i].w;

				// Get text location in screen space, offset a bit and draw.
				glm::vec2 text_pos = persp_camera.worldToScreen(blob_pos, mRenderWindow->getRectPixels());
				text_comp.setLocation(text_pos + glm::vec2(0, 25));
				text_comp.setLineIndex(i);
				text_comp.draw(*mRenderWindow);
			}

			// Draw the GUI
			mGuiService->draw();

			// End render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Signal the ending of the frame
		mRenderService->endFrame();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void FaceDetectionApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void FaceDetectionApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}


	int FaceDetectionApp::shutdown()
	{
		return 0;
	}


	void FaceDetectionApp::updateGUI()
	{
		// General Information
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Application Framerate: %.02f", getCore().getFramerate()).c_str());

		// Show selection box
		static const char* items[] = { "Video", "Webcam" };
		ImGui::Combo("Select", &mCurrentSelection, items, IM_ARRAYSIZE(items));

		// Video controls
		if (ImGui::CollapsingHeader("Video"))
		{
			CVVideo& adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
			if (ImGui::SliderInt("Location", &mCurrentVideoFrame, 0, adapter.geFrameCount()))
				adapter.setFrame(mCurrentVideoFrame);

			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mVideoOutputTexture->getWidth()) / static_cast<float>(mVideoCaptureTexture->getHeight());
			ImGui::Image(*mVideoOutputTexture, { col_width, col_width / ratio_video });

			if (ImGui::Button("Set Streak"))
			{
				CVVideo& video_adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
				utility::ErrorState error;
				if (!video_adapter.changeVideo("streak.mp4", error))
					nap::Logger::info(error.toString());
				mCurrentVideoFrame = 0;
			}

			ImGui::SameLine();
			if (ImGui::Button("Set People"))
			{
				CVVideo& video_adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
				utility::ErrorState error;
				if (!video_adapter.changeVideo("people.mp4", error))
					nap::Logger::info(error.toString());
				mCurrentVideoFrame = 0;
			}
			if (adapter.hasErrors())
			{
				nap::CVCaptureErrorMap map = adapter.getErrors();
				for (auto error : map)
					ImGui::TextColored(clr, error.second.c_str());
			}
		}

		// Webcam controls
		if (ImGui::CollapsingHeader("Webcam"))
		{
			CVCamera& camera_one = mCameraCaptureDevice->getAdapter<CVCamera>(0);
			if (camera_one.hasErrors())
			{
				nap::CVCaptureErrorMap map = camera_one.getErrors();
				for (auto error : map)
					ImGui::TextColored(clr, error.second.c_str());

				if (ImGui::Button("Reconnect Camera One"))
				{
					nap::utility::ErrorState error;
					if (!camera_one.reconnect(error))
						nap::Logger::error(error.toString());
				}
			}
			else
			{
				ImGui::Text("No Errors");
			}

			// Display camera framerate and draw camera texture
			ImGui::Text(utility::stringFormat("Framerate: %.02f", mCameraCaptureDevice->getAdapter<CVCamera>(0).getFPS()).c_str());
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mCameraOutputTexture->getWidth()) / static_cast<float>(mCameraCaptureTexture->getHeight());
			ImGui::Image(*mCameraOutputTexture, { col_width, col_width / ratio_video });
		}
		ImGui::End();
	}
}