#include "auraapp.h"

// Local includes
#include "lineselectioncomponent.h"
#include "lasercontrolcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <inputrouter.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AuraApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool AuraApp::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService	= getCore().getService<RenderService>(); 
		mInputService	= getCore().getService<InputService>();
		mSceneService	= getCore().getService<SceneService>();
		mLaserService	= getCore().getService<EtherDreamService>();
		mOscService		= getCore().getService<OSCService>();
		mGUIService		= getCore().getService<IMGuiService>();

		// Initialize all services

		// Get resource manager service
		mResourceManager = getCore().getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("aura.json", error)) 
			return false;    

		mScene = mResourceManager->findObject<Scene>("Scene");

		// Store all render windows
		mRenderWindow = mResourceManager->findObject<RenderWindow>("Window");

		// Store laser dacs
		mLaserController = mScene->findEntity("LaserControllerEntity");

		// Store camera
		mLaserCamera = mScene->findEntity("LaserCameraEntity");
		assert(mLaserCamera != nullptr);

		// Store frame camera
		mFrameCamera = mScene->findEntity("FrameCameraEntity");
		assert(mFrameCamera != nullptr);

		// Set render states
		RenderState render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;

		mRenderService->setRenderState(render_state);

		// Initialize gui
		mGUI = std::make_unique<AuraGui>(*this);
		mGUI->init();

		return true;
	}
	
	
	// Called when the window is going to render
	void AuraApp::render()
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
		opengl::RenderTarget& backbuffer = mRenderWindow->getBackbuffer();
		mRenderService->clearRenderTarget(backbuffer);

		// Render all laser frames to the window
		PerspCameraComponentInstance& frame_cam = mFrameCamera->getComponent<PerspCameraComponentInstance>();
		laser_control_comp.renderFrames(*mRenderWindow, frame_cam, *mRenderService);

		// Draw our GUI to screen
		mGUI->draw();

		// Swap back buffer
		mRenderWindow->swap();
	}
	
	
	void AuraApp::update(double deltaTime)
	{
		// Forward all window events
		DefaultInputRouter router;
		mInputService->processWindowEvents(*mRenderWindow, router, { mFrameCamera.get() });

		// Update Gui
		mGUI->update(deltaTime);
	}


	void AuraApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void AuraApp::inputMessageReceived(InputEventPtr inputEvent) 
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			switch (press_event->mKey)
			{
			case nap::EKeyCode::KEY_ESCAPE:
				quit();
				break;
			case nap::EKeyCode::KEY_f:
				mRenderWindow->toggleFullscreen();
				break;
			default:
				break;
			}
		}
		mInputService->addEvent(std::move(inputEvent));
	}

	
	int AuraApp::shutdown()
	{
		mGUI.reset(nullptr);
		return 0;
	}
}
 
