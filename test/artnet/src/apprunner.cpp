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
	bool AppRunner::init(Core& core, utility::ErrorState& error)
	{
		// Create all services
		mRenderService = core.getService<RenderService>();
		mInputService  = core.getService<InputService>();
		mSceneService  = core.getService<SceneService>();
		mArtnetService = core.getService<ArtNetService>();
		
		// Load scene
		mResourceManager = core.getResourceManager();
		if (!mResourceManager->loadFile("data/artnet/artnet.json", error))
			return false;
		
		opengl::flush();
		
		// Get important entities
		mCameraEntity = mResourceManager->findEntity("CameraEntity");
		assert(mCameraEntity != nullptr);
		
		// Store all render windows
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));
		
		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void AppRunner::update(double deltaTime)
	{		
		// Update and send our test data over ArtNET
		float sine = sin(mRenderService->getCore().getElapsedTime() * (M_PI * 2));
		
		std::vector<float> channel_data;
		channel_data.resize(512);
		
		for (int index = 0; index < channel_data.size(); ++index)
		{
			float offset = (sine + 1.0f) / 2.0f;
			channel_data[index] = std::min((float)index / (float)channel_data.size() + offset, 1.0f);
		}
		
		ObjectPtr<ArtNetController> universe_0 = mResourceManager->findObject("Universe0");
		universe_0->send(channel_data);
		
		ObjectPtr<ArtNetController> universe_1 = mResourceManager->findObject("Universe1");
		std::reverse(channel_data.begin(), channel_data.end());
		universe_1->send(channel_data);
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


	void AppRunner::registerInputEvent(InputEventPtr inputEvent) 
	{
		mInputService->addEvent(std::move(inputEvent));
	}

	
	void AppRunner::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void AppRunner::shutdown() 
	{
	}
}
