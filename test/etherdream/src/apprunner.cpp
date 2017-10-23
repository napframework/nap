#include "apprunner.h"

// Local includes
#include "lineselectioncomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>

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
		mLaserPrototype = mResourceManagerService->findEntity("LaserPrototypeEntity");
		
		// Store normals mesh
		mNormalsMesh = mResourceManagerService->template findObject<VisualizeNormalsMesh>("NormalsMesh");
		
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
		
		utility::ErrorState error;
		mNormalsMesh->updateNormals(error, true);
	}
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		mRenderService->destroyGLContextResources({ mRenderWindow });
		
		// Activate current window for drawing
		mRenderWindow->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(mRenderWindow->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		
		EntityInstance* spline_entity = mLaserPrototype->getChildren()[0];
		EntityInstance* laser_output_entity = mLaserPrototype->getChildren()[1];
		EntityInstance* camera_entity = mLaserPrototype->getChildren()[2];
		
		// Render spline
		RenderableMeshComponentInstance& line_mesh = spline_entity->getComponent<RenderableMeshComponentInstance>();
		mRenderService->renderObjects(backbuffer, camera_entity->getComponent<PerspCameraComponentInstance>());
		
		// Swap back buffer
		mRenderWindow->swap();
	}
	
	void AppRunner::changeLineSelectionIndex(int changeAmount) {
		EntityInstance* spline_entity = mLaserPrototype->getChildren()[0];
		LineSelectionComponentInstance& line_selection = spline_entity->getComponent<LineSelectionComponentInstance>();
		line_selection.setIndex(line_selection.getIndex() + changeAmount);
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
	
	
	void AppRunner::registerInputEvent(InputEventPtr inputEvent) {
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
