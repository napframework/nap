#include "apprunner.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <inputcomponent.h>

// Mod nap render includes
#include "fractionlayoutcomponent.h"
#include <orthocameracomponent.h>

// Local includes
#include "uiinputrouter.h"
#include "slideshowcomponent.h"

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
		mResourceManager = core.getResourceManager();
		
		// Create render service
		mRenderService = core.getOrCreateService<RenderService>();
		
		utility::ErrorState error;
		if (!mRenderService->init(error))
		{
			Logger::fatal(error.toString());
			return false;
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Input Service
		mInputService = core.getOrCreateService<InputService>();
		//////////////////////////////////////////////////////////////////////////
		
		//////////////////////////////////////////////////////////////////////////
		// Scene service
		//////////////////////////////////////////////////////////////////////////
		mSceneService = core.getOrCreateService<SceneService>();
		
		//////////////////////////////////////////////////////////////////////////
		// Resources
		//////////////////////////////////////////////////////////////////////////
		
		utility::ErrorState errorState;
		if (!mResourceManager->loadFile("data/tommy/tommy.json", errorState))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			return false;
		}
		
		
		mUiInputRouter = mResourceManager->findEntity("UIInputRouterEntity");
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window"));

		// Bind button clicks
		ObjectPtr<EntityInstance> buttonRightEntity = mResourceManager->findEntity("ButtonRightEntity");
		ObjectPtr<EntityInstance> buttonLeftEntity = mResourceManager->findEntity("ButtonLeftEntity");
		buttonRightEntity->getComponent<PointerInputComponentInstance>().pressed.connect(std::bind(&AppRunner::rightButtonClicked, this, std::placeholders::_1));
		buttonLeftEntity->getComponent<PointerInputComponentInstance>().pressed.connect(std::bind(&AppRunner::leftButtonClicked, this, std::placeholders::_1));
		
		// Set render states
		RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Cycle-right button clicked
	void AppRunner::rightButtonClicked(const PointerPressEvent& evt) {
		if (mSlideShowEntity != nullptr && evt.mButton == EMouseButton::LEFT)
		{
			SlideShowComponentInstance& component = mSlideShowEntity->getComponent<SlideShowComponentInstance>();
			component.cycleRight();
		}
	}

	
	// Cycle-left button clicked
	void AppRunner::leftButtonClicked(const PointerPressEvent& evt) {
		if (mSlideShowEntity != nullptr && evt.mButton == EMouseButton::LEFT)
		{
			SlideShowComponentInstance& component = mSlideShowEntity->getComponent<SlideShowComponentInstance>();
			component.cycleLeft();
		}
	}

	
	// Called when the window is updating
	void AppRunner::update()
	{
		// If any changes are detected, and we are reloading, we need to do this on the correct context
		mRenderService->getPrimaryWindow().makeCurrent();
		mResourceManager->checkForFileChanges();
		
		if (mCameraEntity == nullptr)
		{
			mCameraEntity = mResourceManager->findEntity("CameraEntity");
		}
		
		if (mSlideShowEntity == nullptr)
			mSlideShowEntity = mResourceManager->findEntity("SlideShowEntity");
		
		if (mRootLayoutEntity == nullptr)
			mRootLayoutEntity = mResourceManager->findEntity("RootEntity");
		
		if (mCameraEntity != nullptr)
		{
			std::vector<EntityInstance*> entities;
			entities.push_back(&mResourceManager->getRootEntity());
			
			UIInputRouter& router = mUiInputRouter->getComponent<UIInputRouterComponentInstance>().mInputRouter;
			mInputService->processEvents(*mRenderWindows[0], router, entities);
		}
		
		// Process events for all windows
		mRenderService->processEvents();
		
		// Update model transform
		float elapsed_time = mRenderService->getCore().getElapsedTime();
		static float prev_elapsed_time = elapsed_time;
		float delta_time = elapsed_time - prev_elapsed_time;
		if (delta_time < 0.0001f)
		{
			delta_time = 0.0001f;
		}
		
		mResourceManager->getRootEntity().update(delta_time);
		
		if (mRootLayoutEntity != nullptr)
		{
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
		
		// Update the scene
		mSceneService->update();
		
		prev_elapsed_time = elapsed_time;
	}
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		mRenderService->destroyGLContextResources(mRenderWindows);
		
		{
			RenderWindow* render_window = mRenderWindows[0].get();
			
			if (mCameraEntity != nullptr)
			{
				render_window->makeActive();
				
				opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
				render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
				mRenderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);
				
				mRenderService->renderObjects(*render_target, mCameraEntity->getComponent<OrthoCameraComponentInstance>());
				
				render_window->swap();
			}
		}
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
