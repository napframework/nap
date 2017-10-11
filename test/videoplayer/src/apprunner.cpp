#include "apprunner.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <orthocameracomponent.h>
#include "texture2d.h"

namespace nap {
	
	/**
	 * Constructor
	 */
	AppRunner::AppRunner() { }
	

	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool AppRunner::init(Core& core)
	{
		core.initialize();
		
		//////////////////////////////////////////////////////////////////////////
		// GL Service + Window
		//////////////////////////////////////////////////////////////////////////
		
		// Get resource manager service
		mResourceManagerService = core.getOrCreateService<ResourceManagerService>();
		
		// Create render service
		mRenderService = core.getOrCreateService<RenderService>();
		
		utility::ErrorState error;
		if (!mRenderService->init(error))
		{
			Logger::fatal(error.toString());
			return false;
		}
		
		mInputService = core.getOrCreateService<InputService>();
		mSceneService = core.getOrCreateService<SceneService>();
		mVideoService = core.getOrCreateService<VideoService>();
		
		utility::ErrorState errorState;
		if (!mVideoService->init(errorState))
		{
			Logger::fatal("Failed to init video service: \n %s", errorState.toString().c_str());
			return false;
		}
		
		if (!mResourceManagerService->loadFile("data/videoplayer/videoplayer.json", errorState))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			return false;
		}
		
		// Get important entities
		mCameraEntity = mResourceManagerService->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		mVideoEntity = mResourceManagerService->findEntity("VideoEntity");
		assert(mVideoEntity != nullptr);
		
		// Store all render windows
		mRenderWindows.push_back(mResourceManagerService->template findObject<RenderWindow>("Window"));
		
		// Collect all video resources and play
		mVideoResources.push_back(mResourceManagerService->findObject<Video>("Video1"));
		for (auto& videoResource : mVideoResources)
		{
			videoResource->mLoop = true;
			videoResource->play();
		}
		
		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void AppRunner::update()
	{
		// If any changes are detected, and we are reloading, we need to do this on the correct context
		mRenderService->getPrimaryWindow().makeCurrent();
		mResourceManagerService->checkForFileChanges();
		
		// Process events for all windows
		mRenderService->processEvents();
		
		// Update all resources
		mResourceManagerService->update();
		
		// Update model transform
		float elapsed_time = mRenderService->getCore().getElapsedTime();
		static float prev_elapsed_time = elapsed_time;
		float delta_time = elapsed_time - prev_elapsed_time;
		
		if (mVideoEntity != nullptr)
		{
			glm::vec2 window_size = mRenderWindows[0]->getWindow()->getSize();
			
			float new_window_height = -FLT_MAX;
			for (auto& video_resource : mVideoResources)
			{
				utility::ErrorState error_state;
				if (!video_resource->update(delta_time, error_state))
				{
					Logger::fatal(error_state.toString());
				}
				
				//std::cout << video_resource->getTimeStamp() << "\n";
				
				float aspect_ratio = (float)video_resource->getWidth() / (float)video_resource->getHeight();
				new_window_height = std::max(new_window_height, window_size.x / aspect_ratio);
			}
			
			window_size.y = new_window_height;
			mRenderWindows[0]->getWindow()->setSize(window_size);
			
			MaterialInstance& plane_material = mVideoEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
			
			plane_material.getOrCreateUniform<UniformTexture2D>("yTexture").setTexture(mVideoResources[0]->getYTexture());
			plane_material.getOrCreateUniform<UniformTexture2D>("uTexture").setTexture(mVideoResources[0]->getUTexture());
			plane_material.getOrCreateUniform<UniformTexture2D>("vTexture").setTexture(mVideoResources[0]->getVTexture());
			
			// We set the position/size of the root layout element to cover the full screen.
			TransformComponentInstance& transform_component = mVideoEntity->getComponent<TransformComponentInstance>();
			transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
			transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));
		}
		
		// Update the scene
		mSceneService->update();
		
		prev_elapsed_time = elapsed_time;
	}
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		mRenderService->destroyGLContextResources(mRenderWindows);
		
		{
			RenderWindow* render_window = mRenderWindows[0].get();
			
			if (mCameraEntity != nullptr)
			{
				render_window->makeActive();
				
				opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
				render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
				mRenderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);
				
				mRenderService->renderObjects(*render_target, mCameraEntity->getComponent<OrthoCameraComponentInstance>());
				
				render_window->swap();
			}
		}
	}
	

	/**
	 * Handles the window event
	 */
	void AppRunner::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void AppRunner::registerWindowEvent(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	void AppRunner::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManagerService->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void AppRunner::shutdown() 
	{
		mRenderService->shutdown();
	}
}
