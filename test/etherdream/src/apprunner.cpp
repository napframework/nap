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

		// Collects all the errors
		utility::ErrorState errorState;

		// Create input service
		mInputService = core.getOrCreateService<InputService>();

		// Create scene service
		mSceneService = core.getOrCreateService<SceneService>();

		// Create etherdream service
		mLaserService = core.getOrCreateService<EtherDreamService>();
		if (!mLaserService->init(errorState))
		{
			Logger::fatal("unable to create laser service: %s", errorState.toString().c_str());
			return false;
		}

		// Create osc service
		mOscService = core.getOrCreateService<OSCService>();
		if (!mOscService->init(errorState))
		{
			Logger::fatal("unable to create osc service: %s", errorState.toString().c_str());
			return false;
		}

		// Load scene
		if (!mResourceManagerService->loadFile("data/etherdream/etherdream.json", errorState))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			return false; 
		}
		glFlush();

		// Store all render windows
		mRenderWindow = mResourceManagerService->findObject<RenderWindow>("Window");

		// Store laser dacs
		mLaserController = mResourceManagerService->findEntity("LaserControllerEntity");

		// Store camera
		mLaserCamera = mResourceManagerService->findEntity("LaserCameraEntity");
		assert(mLaserCamera != nullptr);

		// Store frame camera
		mFrameCamera = mResourceManagerService->findEntity("FrameCameraEntity");
		assert(mFrameCamera != nullptr);

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

		// Process all events for osc
		mOscService->update();

		// Update the scene
		mSceneService->update();

		// Update all resources
		mResourceManagerService->update();
	}
	
	
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
		mResourceManagerService->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void AppRunner::shutdown() 
	{
		mRenderService->shutdown();
	}
}
