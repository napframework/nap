#include "helloworldapp.h"

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

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HelloWorldApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool HelloWorldApp::init(utility::ErrorState& error)
	{		
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("helloworld.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
		mCameraCaptureDevice = mResourceManager->findObject<nap::CVCaptureDevice>("CameraCaptureDevice");
		mVideoCaptureDevice = mResourceManager->findObject<nap::CVCaptureDevice>("VideoCaptureDevice");
		mCameraTextureOne = mResourceManager->findObject<nap::RenderTexture2D>("CameraTextureOne");
		mCameraTextureTwo = mResourceManager->findObject<nap::RenderTexture2D>("CameraTextureTwo");
		mVideoTexture = mResourceManager->findObject<nap::RenderTexture2D>("VideoTexture");

		// Get the resource that manages all the entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Fetch world and text
		mWorldEntity = scene->findEntity("World");
		mTextEntity = scene->findEntity("Text");

		// Fetch the two different cameras
		mPerspectiveCamEntity = scene->findEntity("PerspectiveCamera");
		mOrthographicCamEntity = scene->findEntity("OrthographicCamera");

		//cv::samples::addSamplesDataSearchPath(utility::)

		//-- 1. Load the cascades
		if (!face_cascade.load("haarcascades/haarcascade_frontalface_alt.xml"))
		{
			std::cout << "--(!)Error loading face cascade\n";
			return false;
		};
		if (!eyes_cascade.load("haarcascades/haarcascade_eye_tree_eyeglasses.xml"))
		{
			std::cout << "--(!)Error loading eyes cascade\n";
			return false;
		};

		// Create a new frame
		CVFrame frame_one;
		frame_one.add(cv::UMat(256, 256, 0));
		frame_one.add(cv::UMat(128, 128, 0));

		// Perform a deep copy
		CVFrame frame_two;
		frame_one.copyTo(frame_two);

		// Perform a weak copy
		CVFrame frame_three(frame_two);

		return true;
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
	void HelloWorldApp::update(double deltaTime)
	{
		if (mCameraCaptureDevice->grab(mCamFrame))
		{
			mCameraCaptureDevice->capture();

			//detectFaces(mCamFrame[0]);
			assert(mCamFrame.getCount() == 2);
			cv::flip(mCamFrame[0][0], mCamFrame[0][0], 0);
			cv::Mat cpu_mat = mCamFrame[0][0].getMat(cv::ACCESS_READ);
			mCameraTextureOne->update(cpu_mat.data);

			cv::flip(mCamFrame[1][0], mCamFrame[1][0], 0);
			cpu_mat = mCamFrame[1][0].getMat(cv::ACCESS_READ);
			mCameraTextureTwo->update(cpu_mat.data);
		}
		
		if (mVideoCaptureDevice->grab(mVidFrame))
		{
			detectFaces(mVidFrame[0]);
			cv::flip(mVidFrame[0][0], mVidFrame[0][0], 0);
			cv::Mat cpu_mat = mVidFrame[0][0].getMat(cv::ACCESS_READ);
			mVideoTexture->update(cpu_mat.data);
		}

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
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Webcam Feed One"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mCameraTextureOne->getWidth()) / static_cast<float>(mCameraTextureOne->getHeight());
			ImGui::Image(*mCameraTextureOne, { col_width, col_width / ratio_video });
		}
		if (ImGui::CollapsingHeader("Webcam Feed Two"))
		{
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mCameraTextureTwo->getWidth()) / static_cast<float>(mCameraTextureTwo->getHeight());
			ImGui::Image(*mCameraTextureTwo, { col_width, col_width / ratio_video });
		}
		if (ImGui::CollapsingHeader("Video Feed"))
		{
			CVVideo& adapter = mVideoCaptureDevice->getAdapter<CVVideo>(0);
			if (ImGui::SliderInt("Location", &mCurrentVideoFrame, 0, adapter.geFrameCount()))
			{
				adapter.setFrame(mCurrentVideoFrame);
				mVideoCaptureDevice->capture();
			}
			float col_width = ImGui::GetContentRegionAvailWidth();
			float ratio_video = static_cast<float>(mVideoTexture->getWidth()) / static_cast<float>(mVideoTexture->getHeight());
			ImGui::Image(*mVideoTexture, { col_width, col_width / ratio_video });
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
	void HelloWorldApp::render()
	{
		// Update the camera location in the world shader for the halo effect
		// To do that we fetch the material associated with the world mesh and query the camera location uniform
		// Once we have the uniform we can set it to the camera world space location
		nap::RenderableMeshComponentInstance& render_mesh = mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformVec3& cam_loc_uniform = render_mesh.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("inCameraPosition");

		nap::TransformComponentInstance& cam_xform = mPerspectiveCamEntity->getComponent<nap::TransformComponentInstance>();
		glm::vec3 global_pos = math::extractPosition(cam_xform.getGlobalTransform());
		cam_loc_uniform.setValue(global_pos);

		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

		// Activate current window for drawing
		mRenderWindow->makeActive();

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

		//cv::imshow("Capture - Face detection", mMat);
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void HelloWorldApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void HelloWorldApp::inputMessageReceived(InputEventPtr inputEvent)
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


	int HelloWorldApp::shutdown()
	{
		return 0;
	}


	void HelloWorldApp::detectFaces(CVFrame& frame)
	{
		cvtColor(frame[0], mMatGS, cv::COLOR_RGB2GRAY);
		equalizeHist(mMatGS, mMatGS);
			
		std::vector<cv::Rect> faces;
		face_cascade.detectMultiScale(mMatGS, faces);

		for (size_t i = 0; i < faces.size(); i++)
		{
			cv::Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
			ellipse(frame[0], center, cv::Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, cv::Scalar(255, 0, 255), 4);
		}
	}
}
