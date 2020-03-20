#include "facedetectionapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <renderable3dtextcomponent.h>
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <scene.h>
#include <perspcameracomponent.h>
#include <inputrouter.h>
#include <imgui/imgui.h>
#include <triangleiterator.h>
#include <meshutils.h>
#include <mathutils.h>
#include <renderableglyph.h>
#include <font.h>
#include <imguiutils.h>
#include <cvframe.h>
#include <rendertotexturecomponent.h>
#include <renderableclassifycomponent.h>

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
		int count = mCVService->getThreadCount();
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

		// Fetch capture OpenCV capture / track entities

		// Fetch the entities
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");
		mOpenCVEntity = scene->findEntity("OpenCV");
		mTextEntity = scene->findEntity("Text");

		CVVideo& adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
		return adapter.changeVideo(adapter.mFile, error);
	}
	
	
	/**
	 * Forward all the received input messages to the camera input components.
	 * The input router is used to filter the input events and to forward them
	 * to the input components of a set of entities, in this case our camera.
	 * 
	 * The camera has two input components: KeyInputComponent and PointerInputComponent
	 * The key input component receives key events, the pointer input component receives pointer events
	 * The orbit controller listens to both of them
	 * When an input component receives a message it sends a signal to the orbit controller.
	 * The orbit controller validates if it's something useful and acts accordingly,
	 * in this case by rotating around or zooming in on the sphere.
	 */
	void FaceDetectionApp::update(double deltaTime)
	{
		// The default input router forwards messages to key and mouse input components
		// attached to a set of entities.
		nap::DefaultInputRouter input_router;
		
		// Forward all input events associated with the first window to the listening components
		std::vector<nap::EntityInstance*> entities = { mPerspectiveCamEntity.get() };
		mInputService->processWindowEvents(*mRenderWindow, input_router, entities);

		// Draw some gui elements
		ImGui::Begin("Controls");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
		ImGui::Text(utility::stringFormat("Application Framerate: %.02f", getCore().getFramerate()).c_str());

		if (ImGui::CollapsingHeader("Webcam Feed"))
		{
			CVCamera* camera_one = mResourceManager->findObject<nap::CVCamera>("Camera").get();
			if (camera_one->hasErrors())
			{
				nap::CVCaptureErrorMap map = camera_one->getErrors();
				for (auto error : map)
				{
					ImGui::TextColored(clr, error.second.c_str());
				}

				if (ImGui::Button("Reconnect Camera One"))
				{
					nap::utility::ErrorState error;
					camera_one->reconnect(error);
				}
			}
			else
				ImGui::Text("No Errors");

			ImGui::Text(utility::stringFormat("Framerate: %.02f", 1.0f / mCameraCaptureDevice->getCaptureTime()).c_str());
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mCameraOutputTexture->getWidth()) / static_cast<float>(mCameraCaptureTexture->getHeight());
			//ImGui::Image(*mCameraCaptureTexture, { col_width, col_width / ratio_video });
			ImGui::Image(*mCameraOutputTexture, { col_width, col_width / ratio_video });
		}
		if (ImGui::CollapsingHeader("Video Feed"))
		{
			CVVideo& adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
			if (ImGui::SliderInt("Location", &mCurrentVideoFrame, 0, adapter.geFrameCount()))
			{
				adapter.setFrame(mCurrentVideoFrame);
			}
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mVideoOutputTexture->getWidth()) / static_cast<float>(mVideoCaptureTexture->getHeight());
			ImGui::Image(*mVideoOutputTexture,  { col_width, col_width / ratio_video });

			if (ImGui::Button("Set Streak"))
			{
				CVVideo& video_adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
				utility::ErrorState error;
				if (!video_adapter.changeVideo("streak.mp4", error))
				{
					nap::Logger::info(error.toString());
				}
				mCurrentVideoFrame = 0;
			}

			ImGui::SameLine();
			if (ImGui::Button("Set People"))
			{
				CVVideo& video_adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
				utility::ErrorState error;
				if (!video_adapter.changeVideo("people.mp4", error))
				{
					nap::Logger::info(error.toString());
				}
				mCurrentVideoFrame = 0;
			}

			if (adapter.hasErrors())
			{
				nap::CVCaptureErrorMap map = adapter.getErrors();
				for (auto error : map)
				{
					ImGui::TextColored(clr, error.second.c_str());
				}
			}
		}
		ImGui::End();
	}

	
	/**
	 * Render loop is rather straight forward:
	 * Set the camera position in the world shader for the halo effect
	 * make the main window active, this makes sure that all subsequent render calls are 
	 * associated with that window. When you have multiple windows and don't activate the right window subsequent
	 * render calls could end up being associated with the wrong context, resulting in undefined behavior.
	 * Next we clear the render target, render the world and after that the text. 
	 * Finally we swap the main window back-buffer, making sure the rendered image is blitted to screen.
	 */
	void FaceDetectionApp::render()
	{
		// Activate current (main) window for drawing
		mRenderWindow->makeActive();

		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

		// Render detected blobs to texture for all OpenCV capture entities
		{
			for (auto& entity : mOpenCVEntity->getChildren())
			{
				entity->getComponent<RenderToTextureComponentInstance>().draw();
			}
		}

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Find the perspective camera
		nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

		// Find objects to render
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		
		// Render detected blobs + ground plane to viewport for the selected OpenCV capture entity.
		nap::EntityInstance& capture_entity = (*mOpenCVEntity)[0];
		for (auto& entity : capture_entity[0].getChildren())
		{
			RenderableComponentInstance& render_comp = entity->getComponent<RenderableComponentInstance>();
			components_to_render.emplace_back(&render_comp);
		}

		// Render the world with the right camera directly to screen
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), persp_camera, components_to_render);

		// Get renderable 2D text component
		Renderable2DTextComponentInstance& text_comp = mTextEntity->getComponent<Renderable2DTextComponentInstance>();

		// Get blob location
		nap::EntityInstance& blob_entity = (*mOpenCVEntity)[0][0][0];
		RenderableClassifyComponentInstance& render_comp = blob_entity.getComponent<RenderableClassifyComponentInstance>();
		const std::vector<glm::mat4>& locs = render_comp.getLocations();
		const std::vector<float>& sizes = render_comp.getSizes();

		utility::ErrorState error;
		for (int i = 0; i < locs.size(); i++)
		{
			// Get blob location in 3D
			glm::vec3 blob_pos = math::extractPosition(locs[i]); 
			blob_pos.y += sizes[i];
			
			// Get text location in screen space
			glm::vec2 text_pos = persp_camera.worldToScreen(blob_pos, mRenderWindow->getRectPixels());
			text_comp.setLocation(text_pos + glm::vec2(0,25));
			text_comp.setText(utility::stringFormat("Blob %d", i+1), error);
			text_comp.draw(mRenderWindow->getBackbuffer());
		}

		// Draw our gui
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
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
}
