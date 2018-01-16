#include "videomodulationapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <orthocameracomponent.h>
#include <texture2d.h>
#include <scene.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool VideoApp::init(utility::ErrorState& error)
	{	
		// Get resource manager service
		mResourceManager = getCore().getResourceManager();
		
		// Create render service
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		mVideoService  = getCore().getService<VideoService>();
		
		if (!mResourceManager->loadFile("data/videomodulation/videomodulation.json", error))
			return false;
		
		// Get important entities
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = scene->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		mVideoEntity = scene->findEntity("VideoEntity");
		assert(mVideoEntity != nullptr);
		
		// Store all render windows
		mRenderWindows.push_back(mResourceManager->template findObject<RenderWindow>("Window"));
		
		// Collect all video resources and play
		mVideoResource = mResourceManager->findObject<Video>("Video1");
		assert(mVideoResource != nullptr);
		mVideoResource->mLoop = true;
		mVideoResource->play();
		
		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void VideoApp::update(double deltaTime)
	{				
		glm::vec2 window_size = mRenderWindows[0]->getWindow()->getSize();

		// Fix window width / height
		float new_window_height = -FLT_MAX;

		float aspect_ratio = (float)mVideoResource->getWidth() / (float)mVideoResource->getHeight();
		new_window_height = std::max(new_window_height, window_size.x / aspect_ratio);

		window_size.y = new_window_height;
		mRenderWindows[0]->getWindow()->setSize(window_size);

		MaterialInstance& plane_material = mVideoEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();

		plane_material.getOrCreateUniform<UniformTexture2D>("yTexture").setTexture(mVideoResource->getYTexture());
		plane_material.getOrCreateUniform<UniformTexture2D>("uTexture").setTexture(mVideoResource->getUTexture());
		plane_material.getOrCreateUniform<UniformTexture2D>("vTexture").setTexture(mVideoResource->getVTexture());

		// We set the position/size of the root layout element to cover the full screen.
		TransformComponentInstance& transform_component = mVideoEntity->getComponent<TransformComponentInstance>();
		transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
		transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));
	}
	
	
	// Called when the window is going to render
	void VideoApp::render()
	{
		mRenderService->destroyGLContextResources(mRenderWindows);

		// Make render window active for drawing
		RenderWindow* render_window = mRenderWindows[0].get();
		render_window->makeActive();

		// Clear target
		opengl::RenderTarget& render_target = render_window->getBackbuffer();
		render_target.setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		mRenderService->clearRenderTarget(render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

		// Render necessary objects
		mRenderService->renderObjects(render_target, mCameraEntity->getComponent<OrthoCameraComponentInstance>());

		// Swap GPU buffers
		render_window->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void VideoApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void VideoApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	void VideoApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit(0);

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				static bool fullscreen = true;
				setWindowFullscreen("Window", fullscreen);
				fullscreen = !fullscreen;
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	void VideoApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void VideoApp::shutdown() 
	{

	}
}
