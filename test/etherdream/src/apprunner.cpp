#include "apprunner.h"

// Local includes
#include "lineselectioncomponent.h"
#include "lasercontrolcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>

namespace nap 
{
	
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
		core.initializeEngine();

		// Create render service
		mRenderService = core.getOrCreateService<RenderService>();
		mInputService = core.getOrCreateService<InputService>();
		mSceneService = core.getOrCreateService<SceneService>();
		mLaserService = core.getOrCreateService<EtherDreamService>();
		mOscService = core.getOrCreateService<OSCService>();

		// Initialize all services
		utility::ErrorState errorState;
		if (!core.initializeServices(errorState))
		{
			Logger::fatal("unable to initialize services: %s", errorState.toString().c_str());
			return false;
		}

		// Get resource manager service
		mResourceManager = core.getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("data/etherdream/etherdream.json", errorState))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			return false;
		}
		glFlush();

		// Store all render windows
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Store laser dacs
		mLaserController = mResourceManager->findEntity("LaserControllerEntity");

		// Store camera
		mLaserCamera = mResourceManager->findEntity("LaserCameraEntity");
		assert(mLaserCamera != nullptr);

		// Store frame camera
		mFrameCamera = mResourceManager->findEntity("FrameCameraEntity");
		assert(mFrameCamera != nullptr);

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;

		return true;
	}
	
	
	// Called when the window is updating
	void AppRunner::update(double deltaTime)
	{ }
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		// Get rid of unnecessary resources
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Render all lasers objects in to their respective back-buffer
		LaserControlInstanceComponent& laser_control_comp = mLaserController->getComponent<LaserControlInstanceComponent>();
		PerspCameraComponentInstance& laser_cam = mLaserCamera->getComponent<PerspCameraComponentInstance>();
		laser_control_comp.renderToLaserBuffers(laser_cam, *mRenderService);

		// Clear window back-buffer
		opengl::RenderTarget& backbuffer = *(mRenderWindow->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer);

		// Render all laser frames to the window
		PerspCameraComponentInstance& frame_cam = mFrameCamera->getComponent<PerspCameraComponentInstance>();
		laser_control_comp.renderFrames(*mRenderWindow, frame_cam, *mRenderService);

		// Swap back buffer
		mRenderWindow->swap();
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
		mRenderService->shutdown();
	}
}
