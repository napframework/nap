#include "@PROJECT_NAME_LOWERCASE@app.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <inputcomponent.h>

// Mod nap render includes
#include <orthocameracomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::@PROJECT_NAME_CAMELCASE@App)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool @PROJECT_NAME_CAMELCASE@App::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		
		// Get resource manager service
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("data/@PROJECT_NAME_LOWERCASE@/appStructure.json", error))
			return false;

		mScene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = mScene->findEntity("CameraEntity");
		
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void @PROJECT_NAME_CAMELCASE@App::update(double deltaTime)
	{
		// If any changes are detected, and we are reloading, we need to do this on the correct context
		mRenderService->getPrimaryWindow().makeCurrent();
		mResourceManager->checkForFileChanges();
	}
	
	
	// Called when the window is going to render
	void @PROJECT_NAME_CAMELCASE@App::render()
	{
		// Make render window active
		mRenderService->destroyGLContextResources(mRenderWindows);
		RenderWindow* render_window = mRenderWindows[0].get();
		render_window->makeActive();

		// Set target
		opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
		render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		mRenderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

		// Render objects
		mRenderService->renderObjects(*render_target, mCameraEntity->getComponent<OrthoCameraComponentInstance>());
		
		render_window->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void @PROJECT_NAME_CAMELCASE@App::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void @PROJECT_NAME_CAMELCASE@App::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void @PROJECT_NAME_CAMELCASE@App::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit(0);
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	void @PROJECT_NAME_CAMELCASE@App::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void @PROJECT_NAME_CAMELCASE@App::shutdown()
	{

	}
}
