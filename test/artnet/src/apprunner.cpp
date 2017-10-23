#include "apprunner.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>


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
		mResourceManagerService = core.getResourceManager();
		
		// Create render service
		mRenderService = core.getOrCreateService<RenderService>();
		
		utility::ErrorState error;
		if (!mRenderService->init(error))
		{
			Logger::fatal(error.toString());
			return false;
		}
		
		// Collects all the errors
		utility::ErrorState errorState;
		
		//////////////////////////////////////////////////////////////////////////
		// Services
		//////////////////////////////////////////////////////////////////////////
		
		// Create input service
		mInputService = core.getOrCreateService<InputService>();
		
		// Create scene service
		mSceneService = core.getOrCreateService<SceneService>();
		
		// Create artnet service
		mArtnetService = core.getOrCreateService<ArtNetService>();

		
		//////////////////////////////////////////////////////////////////////////
		// Resources
		//////////////////////////////////////////////////////////////////////////
		
		// Load scene
		if (!mResourceManagerService->loadFile("data/artnet/artnet.json", errorState))
		{
			Logger::fatal("Unable to de-serialize resources: \n %s", errorState.toString().c_str());
			return false;
		}
		
		glFlush();
		
		// Get important entities
		mCameraEntity = mResourceManagerService->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		// Store all render windows
		mRenderWindows.push_back(mResourceManagerService->findObject<RenderWindow>("Window"));
		
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
		
		// Update and send our test data over ArtNET
		float sine = sin(mRenderService->getCore().getElapsedTime() * (M_PI * 2));
		
		std::vector<float> channel_data;
		channel_data.resize(512);
		
		for (int index = 0; index < channel_data.size(); ++index)
		{
			float offset = (sine + 1.0f) / 2.0f;
			channel_data[index] = std::min((float)index / (float)channel_data.size() + offset, 1.0f);
		}
		
		ObjectPtr<ArtNetController> universe_0 = mResourceManagerService->findObject("Universe0");
		universe_0->send(channel_data);
		
		ObjectPtr<ArtNetController> universe_1 = mResourceManagerService->findObject("Universe1");
		std::reverse(channel_data.begin(), channel_data.end());
		universe_1->send(channel_data);
		
		// Update the scene
		mSceneService->update();
		
		// Update ArtNET
		mArtnetService->update();
	}
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		mRenderService->destroyGLContextResources(mRenderWindows);
		
		// Activate current window for drawing
		mRenderWindows[0]->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(mRenderWindows[0]->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		
		// Swap backbuffer
		mRenderWindows[0]->swap();
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
