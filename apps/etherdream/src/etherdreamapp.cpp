#include "etherdreamapp.h"

// Local includes
#include "lineselectioncomponent.h"
#include "lasercontrolcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include "nap/scene.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EtherdreamApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool EtherdreamApp::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>(); 
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		mLaserService  = getCore().getService<EtherDreamService>();
		mOscService	   = getCore().getService<OSCService>();

		// Initialize all services

		// Get resource manager service
		mResourceManager = getCore().getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("data/etherdream/etherdream.json", error))
			return false;    

		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Store all render windows
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Store laser dacs
		mLaserController = scene->findEntity("LaserControllerEntity");

		// Store camera
		mLaserCamera = scene->findEntity("LaserCameraEntity");
		assert(mLaserCamera != nullptr);

		// Store frame camera
		mFrameCamera = scene->findEntity("FrameCameraEntity");
		assert(mFrameCamera != nullptr);

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;

		return true;
	}
	
	
	// Called when the window is going to render
	void EtherdreamApp::render()
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
	
	
	void EtherdreamApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void EtherdreamApp::inputMessageReceived(InputEventPtr inputEvent) 
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

	
	void EtherdreamApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void EtherdreamApp::shutdown()
	{

	}
}
 