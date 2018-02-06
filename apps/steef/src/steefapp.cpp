#include "steefapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderwindow.h>
#include <scene.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SteefApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{	
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool SteefApp::init(utility::ErrorState& error)
	{
		// Create services
		mRenderService = getCore().getService<nap::RenderService>();
		mSceneService  = getCore().getService<nap::SceneService>();
		mInputService  = getCore().getService<nap::InputService>();
		
		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("objects.json", error))
		{
			assert(false);
			return false;
		}
        
		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Viewport");
		mRenderWindow->mWindowEvent.connect(std::bind(&SteefApp::handleWindowEvent, this, std::placeholders::_1));
		
		// Get vintl textures
		mVinylLabelImg = mResourceManager->findObject<nap::ImageFromFile>("LabelImage");
		mVinylCoverImg = mResourceManager->findObject<nap::ImageFromFile>("CoverImage");
		
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Get entity that holds vinyl
		mModelEntity = scene->findEntity("ModelEntity");
		
		// Get entity that holds the background image
		mBackgroundEntity = scene->findEntity("BackgroundEntity");
		
		// Get entity that holds the camera
		mCameraEntity = scene->findEntity("CameraEntity");
		
		// Set render states
		nap::RenderState& render_state = mRenderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;
		
		return true;
	}
	
	
	// Called when the window is updating
	void SteefApp::update(double deltaTime)
	{
		// Make sure background image matches window size
		updateBackgroundImage();
		
		// Update our shader variables
		updateShader();
		
	}
	
	
	// Called when the window is going to render
	void SteefApp::render()
	{
		
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow });
		
		// Activate current window for drawing
		mRenderWindow->makeActive();
		
		// Clear back-buffer
		opengl::RenderTarget& backbuffer = mRenderWindow->getBackbuffer();
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
	void SteefApp::handleWindowEvent(const nap::WindowEvent& windowEvent)
	{
		nap::rtti::TypeInfo e_type = windowEvent.get_type();
		if (e_type.is_derived_from(RTTI_OF(nap::WindowResizedEvent)) ||
			e_type.is_derived_from(RTTI_OF(nap::WindowShownEvent)))
		{
			updateBackgroundImage();
		}
	}
	
	
	// Forward the window message to the render service
	void SteefApp::windowMessageReceived(WindowEventPtr windowEvent) 
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	// Forward the input message to the input service
	void SteefApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				static bool fullscreen = true;
				setWindowFullscreen("Viewport", fullscreen);
				fullscreen = !fullscreen;
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}

	
	// Make window fullscreen
	void SteefApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<nap::RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	int SteefApp::shutdown() 
	{
		return 0;
	}
	
	
	/**
	 * updates the background image to match the size of the output window
	 */
	void SteefApp::updateBackgroundImage()
	{
		// Get size
		glm::ivec2 window_size = mRenderWindow->getWindow()->getSize();
		
		// Now update background texture
		nap::TransformComponentInstance& xform_comp = mBackgroundEntity->getComponent<nap::TransformComponentInstance>();
		xform_comp.setScale(glm::vec3(window_size.x, window_size.y*-1.0f, 1.0f));
		xform_comp.setTranslate(glm::vec3(float(window_size.x) / 2.0f, float(window_size.y) / 2.0f, -900.0f));
	}
	
	
	void SteefApp::updateShader()
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
