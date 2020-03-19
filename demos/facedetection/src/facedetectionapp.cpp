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

		// Fetch world and text
		mWorldEntity = scene->findEntity("World");
		mTextEntity = scene->findEntity("Text");

		// Fetch capture OpenCV capture / track entities
		mCameraCaptureEntity = scene->findEntity("OpenCVCamera");
		mVideoCaptureEntity  = scene->findEntity("OpenCVVideo");

		// Fetch the two different cameras
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");

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
		ImGui::Text(utility::stringFormat("Cam Framerate: %.02f", 1.0f / mCameraCaptureDevice->getCaptureTime()).c_str());
		ImGui::Text(utility::stringFormat("Vid Framerate: %.02f", 1.0f / mVideoCaptureDevice->getCaptureTime()).c_str());

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
			//ImGui::Image(*mVideoCaptureTexture, { col_width, col_width / ratio_video });
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

		// Render detected blobs to texture
		{
			mCameraCaptureEntity->getComponent<RenderToTextureComponentInstance>().draw();
			mVideoCaptureEntity->getComponent<RenderToTextureComponentInstance>().draw();
		}

		// Update the camera location in the world shader for the halo effect
		// To do that we fetch the material associated with the world mesh and query the camera location uniform
		// Once we have the uniform we can set it to the camera world space location
		nap::RenderableMeshComponentInstance& render_mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformVec3& cam_loc_uniform = render_mesh.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("inCameraPosition");

		nap::TransformComponentInstance& cam_xform = mPerspectiveCamEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
		cam_loc_uniform.setValue(global_pos);

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Find the world and add as an object to render
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		nap::RenderableMeshComponentInstance& renderable_world = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();

		components_to_render.emplace_back(&renderable_world);

		// Find the perspective camera
		nap::PerspCameraComponentInstance& persp_camera = mPerspectiveCamEntity->getComponent<nap::PerspCameraComponentInstance>();

		// Render the world with the right camera directly to screen
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), persp_camera, components_to_render);

		// Clear list of components to render
		components_to_render.clear();

		// Render text on top of the sphere
		Renderable2DTextComponentInstance& render_text = mTextEntity->getComponent<nap::Renderable2DTextComponentInstance>();

		// Find the orthographic camera (2D text can only be rendered with an orthographic camera)
		nap::OrthoCameraComponentInstance& ortho_camera = mOrthographicCamEntity->getComponent<nap::OrthoCameraComponentInstance>();

		// Center text
        render_text.setLocation({ mRenderWindow->getWidthPixels() / 2, mRenderWindow->getHeightPixels() / 2 });

		// Render text on top of sphere using render service
		// Alternatively you can use: render_text.draw(const opengl::BackbufferRenderTarget& target) directly
		components_to_render.emplace_back(&render_text);
		mRenderService->renderObjects(mRenderWindow->getBackbuffer(), ortho_camera, components_to_render);

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
