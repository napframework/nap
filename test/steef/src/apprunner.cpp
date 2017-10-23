#include "apprunner.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>


namespace nap {
	
	/**
	 * Constructor
	 */
	AppRunner::AppRunner() { }
	

	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool AppRunner::init(nap::Core& core)
	{
		core.initialize();
		
		//////////////////////////////////////////////////////////////////////////
		// GL Service + Window
		//////////////////////////////////////////////////////////////////////////
		
		// Get resource manager service
		mResourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
		
		// Create render service
		mRenderService = core.getOrCreateService<nap::RenderService>();
		
		nap::utility::ErrorState error;
		if (!mRenderService->init(error))
		{
			nap::Logger::fatal(error.toString());
			return false;
		}
		nap::Logger::info("initialized render service: %s", mRenderService->getTypeName().c_str());

		//////////////////////////////////////////////////////////////////////////
		// Scene
		//////////////////////////////////////////////////////////////////////////
		mSceneService = core.getOrCreateService<nap::SceneService>();		
		
		//////////////////////////////////////////////////////////////////////////
		// Resources
		//////////////////////////////////////////////////////////////////////////
		
		nap::utility::ErrorState errorState;
		if (!mResourceManagerService->loadFile("data/steef/objects.json", errorState))
		{
			nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
			
			assert(false);
			return false;
		}
        
        glFlush();
		
		// Extract loaded resources
		mRenderWindow = mResourceManagerService->findObject<nap::RenderWindow>("Viewport");
		mRenderWindow->onWindowEvent.connect(std::bind(&AppRunner::handleWindowEvent, this, std::placeholders::_1));
		
		// Get vintl textures
		mVinylLabelImg = mResourceManagerService->findObject<nap::Image>("LabelImage");
		mVinylCoverImg = mResourceManagerService->findObject<nap::Image>("CoverImage");
		
		// Get entity that holds vinyl
		mModelEntity = mResourceManagerService->findEntity("ModelEntity");
		
		// Get entity that holds the background image
		mBackgroundEntity = mResourceManagerService->findEntity("BackgroundEntity");
		
		// Get entity that holds the camera
		mCameraEntity = mResourceManagerService->findEntity("CameraEntity");
		
		// Set render states
		nap::RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void AppRunner::update()
	{
		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(mCameraEntity.get());
		
		// Process events for all windows
		mRenderService->processEvents();
		
		// Need to make primary window active before reloading files, as renderer resources need to be created in that context
		mRenderService->getPrimaryWindow().makeCurrent();
		
		// Reload all files if data changed
		mResourceManagerService->checkForFileChanges();
		
		// Tick for all components that are listening
		mResourceManagerService->update();
		
		// Update the scene
		mSceneService->update();
		
		// Make sure background image matches window size
		updateBackgroundImage();
		
		// Update our shader variables
		updateShader();
		
	}
	
	
	// Called when the window is going to render
	void AppRunner::render()
	{
		
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });
		
		// Activate current window for drawing
		mRenderWindow->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(mRenderWindow->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
		mRenderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		
		// Render Background
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(&(mBackgroundEntity->getComponent<nap::RenderableMeshComponentInstance>()));
		mRenderService->renderObjects(backbuffer, mCameraEntity->getComponent<nap::OrthoCameraComponentInstance>(), components_to_render);
		
		// Render Vinyl
		components_to_render.clear();
		for (const nap::EntityInstance* e : mModelEntity->getChildren())
		{
			if (e->hasComponent<nap::RenderableMeshComponentInstance>())
				components_to_render.emplace_back(&(e->getComponent<nap::RenderableMeshComponentInstance>()));
		}
		mRenderService->renderObjects(backbuffer, mCameraEntity->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);
		
		// Update gpu frame
		mRenderWindow->swap();
	}
	

	/**
	 * Handles the window event
	 * When the window size changes we want to update the background texture to reflect those changes, ie:
	 * Scale to the right size
	 */
	void AppRunner::handleWindowEvent(const nap::WindowEvent& windowEvent)
	{
		nap::rtti::TypeInfo e_type = windowEvent.get_type();
		if (e_type.is_derived_from(RTTI_OF(nap::WindowResizedEvent)) ||
			e_type.is_derived_from(RTTI_OF(nap::WindowShownEvent)))
		{
			updateBackgroundImage();
		}
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
		mResourceManagerService->findObject<nap::RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void AppRunner::shutdown() 
	{
		mRenderService->shutdown();
	}
	
	
	/**
	 * updates the background image to match the size of the output window
	 */
	void AppRunner::updateBackgroundImage()
	{
		// Get size
		glm::ivec2 window_size = mRenderWindow->getWindow()->getSize();
		
		// Now update background texture
		nap::TransformComponentInstance& xform_comp = mBackgroundEntity->getComponent<nap::TransformComponentInstance>();
		xform_comp.setScale(glm::vec3(window_size.x, window_size.y*-1.0f, 1.0f));
		xform_comp.setTranslate(glm::vec3(float(window_size.x) / 2.0f, float(window_size.y) / 2.0f, -900.0f));
	}
	
	
	void AppRunner::updateShader()
	{
		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		// Set camera location on shader, used for rendering highlights
		for (const nap::EntityInstance* e : mModelEntity->getChildren())
		{
			nap::RenderableMeshComponentInstance* mesh = e->findComponent<nap::RenderableMeshComponentInstance>();
			if(mesh == nullptr)
				continue;
			
			nap::UniformVec3& cameraLocation = mesh->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("cameraLocation");
			cameraLocation.setValue(cam_xform.getTranslate());
		}
	}
}
