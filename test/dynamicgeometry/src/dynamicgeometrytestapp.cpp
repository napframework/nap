#include "dynamicgeometrytestapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <scene.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DynamicGeometryTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool DynamicGeometryTestApp::init(utility::ErrorState& error)
	{		
		// Create render service
		mRenderService = getCore().getService<RenderService>();		
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("data/dynamicgeometry/objects.json", error))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", error.toString().c_str());
			return false;   
		}
		
		ObjectPtr<Scene> scene		= mResourceManager->findObject<Scene>("Scene");
		mRenderWindow				= mResourceManager->findObject<RenderWindow>("Window0");
		mCameraEntity				= scene->findEntity("CameraEntity");
		mDefaultInputRouter			= scene->findEntity("DefaultInputRouterEntity");

		// Set render states
		nap::RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;

		return true;
	}
	
	
	// Called when the window is updating
	void DynamicGeometryTestApp::update(double deltaTime)
	{
		DefaultInputRouter& input_router = mDefaultInputRouter->getComponent<DefaultInputRouterComponentInstance>().mInputRouter;
		{
			// Update input for first window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntity.get());

			Window* window = mRenderWindow.get();
			mInputService->processEvents(*window, input_router, entities);
		}
	}
	
	
	// Called when the window is going to render
	void DynamicGeometryTestApp::render()
	{
		// Get rid of unnecessary resources
		mRenderService->destroyGLContextResources({ mRenderWindow });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear window back-buffer
		opengl::RenderTarget& backbuffer = *(mRenderWindow->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer);

		PerspCameraComponentInstance& frame_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();
		mRenderService->renderObjects(backbuffer, frame_cam);

		// Swap back buffer
		mRenderWindow->swap();
	}
	

	/**
	 * Handles the window event
	 */
	void DynamicGeometryTestApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
	}
	
	
	void DynamicGeometryTestApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void DynamicGeometryTestApp::inputMessageReceived(InputEventPtr inputEvent)
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

	
	void DynamicGeometryTestApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void DynamicGeometryTestApp::shutdown() 
	{
	}
}
