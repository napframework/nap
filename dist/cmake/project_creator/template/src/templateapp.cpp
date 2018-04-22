#include "@PROJECT_NAME_LOWERCASE@app.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <inputcomponent.h>

// Mod nap render includes
#include <orthocameracomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::@PROJECT_NAME_PASCALCASE@App)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool @PROJECT_NAME_PASCALCASE@App::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		
		// Get resource manager service
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("appStructure.json", error))
			return false;

		mScene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = mScene->findEntity("CameraEntity");
		
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		
		return true;
	}
	
	
	// Called when the window is updating
	void @PROJECT_NAME_PASCALCASE@App::update(double deltaTime)
	{
		// If any changes are detected, and we are reloading, we need to do this on the correct context
		mRenderService->getPrimaryWindow().makeCurrent();
		mResourceManager->checkForFileChanges();
	}
	
	
	// Called when the window is going to render
	void @PROJECT_NAME_PASCALCASE@App::render()
	{
		// Make render window active
		mRenderService->destroyGLContextResources(mRenderWindows);
		RenderWindow* render_window = mRenderWindows[0].get();
		render_window->makeActive();
	}
	

	/**
	 * Handles the window event
	 */
	void @PROJECT_NAME_PASCALCASE@App::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void @PROJECT_NAME_PASCALCASE@App::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void @PROJECT_NAME_PASCALCASE@App::inputMessageReceived(InputEventPtr inputEvent)
	{
		
	}

	
	void @PROJECT_NAME_PASCALCASE@App::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	int @PROJECT_NAME_PASCALCASE@App::shutdown()
	{
		return 0;
	}

}
