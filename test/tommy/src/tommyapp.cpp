#include "tommyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <inputcomponent.h>
#include <nap/datapathmanager.h>

// Mod nap render includes
#include <orthocameracomponent.h>

// Local includes
#include "uiinputrouter.h"
#include "slideshowcomponent.h"
#include "fractionlayoutcomponent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TommyApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool TommyApp::init(utility::ErrorState& error)
	{
		// Create render service
		mRenderService = getCore().getService<RenderService>();
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();
		
		// Get resource manager service
		mResourceManager = getCore().getResourceManager();
		
		if (!mResourceManager->loadFile(DataPathManager::get().getDataPath() + "tommy.json", error))
			return false;
		
		mScene = mResourceManager->findObject<Scene>("Scene");
		mCameraEntity = mScene->findEntity("CameraEntity");
		mSlideShowEntity = mScene->findEntity("SlideShowEntity");
		mRootLayoutEntity = mScene->findEntity("RootEntity");

		mUiInputRouter = mScene->findEntity("UIInputRouterEntity");
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));

		// Bind button clicks
		ObjectPtr<EntityInstance> buttonRightEntity = mScene->findEntity("ButtonRightEntity");
		ObjectPtr<EntityInstance> buttonLeftEntity = mScene->findEntity("ButtonLeftEntity");
		buttonRightEntity->getComponent<PointerInputComponentInstance>().pressed.connect(std::bind(&TommyApp::rightButtonClicked, this, std::placeholders::_1));
		buttonLeftEntity->getComponent<PointerInputComponentInstance>().pressed.connect(std::bind(&TommyApp::leftButtonClicked, this, std::placeholders::_1));

		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Cycle-right button clicked
	void TommyApp::rightButtonClicked(const PointerPressEvent& evt) {
		if (mSlideShowEntity != nullptr && evt.mButton == EMouseButton::LEFT)
		{
			SlideShowComponentInstance& component = mSlideShowEntity->getComponent<SlideShowComponentInstance>();
			component.cycleRight();
		}
	}

	
	// Cycle-left button clicked
	void TommyApp::leftButtonClicked(const PointerPressEvent& evt) {
		if (mSlideShowEntity != nullptr && evt.mButton == EMouseButton::LEFT)
		{
			SlideShowComponentInstance& component = mSlideShowEntity->getComponent<SlideShowComponentInstance>();
			component.cycleLeft();
		}
	}

	
	// Called when the window is updating
	void TommyApp::update(double deltaTime)
	{
		// If any changes are detected, and we are reloading, we need to do this on the correct context
		mRenderService->getPrimaryWindow().makeCurrent();
		mResourceManager->checkForFileChanges();
		
		std::vector<EntityInstance*> entities;
		entities.push_back(&mScene->getRootEntity());
			
		UIInputRouter& router = mUiInputRouter->getComponent<UIInputRouterComponentInstance>().mInputRouter;
		mInputService->processEvents(*mRenderWindows[0], router, entities);

		glm::vec2 window_size = mRenderWindows[0]->getWindow()->getSize();
			
		// First layout element. We start at -1000.0f, a value in front of the camera that is 'far away'
		// We set the position/size of the root layout element to cover the full screen.
		TransformComponentInstance& transform_component = mRootLayoutEntity->getComponent<TransformComponentInstance>();
		transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
		transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));
			
		// Update the layout
		FractionLayoutComponentInstance& layout = mRootLayoutEntity->getComponent<FractionLayoutComponentInstance>();
		layout.updateLayout(window_size, glm::mat4(1.0f));
	}
	
	
	// Called when the window is going to render
	void TommyApp::render()
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
	void TommyApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void TommyApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void TommyApp::inputMessageReceived(InputEventPtr inputEvent)
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

	
	void TommyApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void TommyApp::shutdown() 
	{

	}
}
