#include "vinylapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderwindow.h>
#include <scene.h>
#include <imgui/imgui.h>

// Register app with RTTI, ensures the app running knows it's running an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VinylApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{	
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool VinylApp::init(utility::ErrorState& error)
	{
		// fetch services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();
		
		// Get resource manager and load our app
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("data/vinyl/vinyl.json", error))
			return false;
        
		// Extract loaded resources and listen to window resize events
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Viewport");
		mRenderWindow->mWindowEvent.connect(std::bind(&VinylApp::handleWindowEvent, this, std::placeholders::_1));
		
		// Fetch vinyl textures
		mVinylLabelImg = mResourceManager->findObject<nap::Image>("LabelImage");
		mVinylCoverImg = mResourceManager->findObject<nap::Image>("CoverImage");
		
		// Fetch scene
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
	void VinylApp::update(double deltaTime)
	{
		// Make sure background image matches window size
		positionBackground();
		
		// Update our shader variables
		setCameraLocation();

		// Set vinyl color
		ImGui::Begin("Controls");
		ImGui::Text("'f'=fullscreen, 'esc'=quit");
		if (ImGui::ColorPicker3("Vinyl Color", mRecordColor.getData()))
		{
			setRecordColor();
		}
		ImGui::End();
	}
	
	
	/**
	 * Render the background using an orthographic camera
	 * Render the vinyl + sleeve using a perspective camera
	 * Regarding lights: the light position is defined as a uniform in JSON
	 * You could make a light and push the changes programmatically to the shader,
	 * But since we only have 2 objects it's easy enough to just specify them individually
	 */
	void VinylApp::render()
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
		
		// Tell the gui to draw
		mGuiService->render();

		// Update gpu frame
		mRenderWindow->swap();
	}
	

	/**
	 * Handles the window event
	 * When the window size changes we want to update the background texture to reflect those changes, ie:
	 * Scale to the right size
	 */
	void VinylApp::handleWindowEvent(const nap::WindowEvent& windowEvent)
	{
		nap::rtti::TypeInfo e_type = windowEvent.get_type();
		if (e_type.is_derived_from(RTTI_OF(nap::WindowResizedEvent)) || e_type.is_derived_from(RTTI_OF(nap::WindowShownEvent)))
		{
			positionBackground();
		}
	}
	
	
	// Forward the window message to the render service
	void VinylApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}

	
	// Forward the input message to the input service
	void VinylApp::inputMessageReceived(InputEventPtr inputEvent)
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
				setWindowFullscreen("Viewport", fullscreen);
				fullscreen = !fullscreen;
			}
		}
	}

	
	// Make window fullscreen
	void VinylApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen)
	{
		mResourceManager->findObject<nap::RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	void VinylApp::shutdown()
	{ }
	
	
	/**
	 * updates the background image to match the size of the output window
	 */
	void VinylApp::positionBackground()
	{
		// Get size
		glm::ivec2 window_size = mRenderWindow->getWindow()->getSize();
		
		// Now update background texture
		nap::TransformComponentInstance& xform_comp = mBackgroundEntity->getComponent<nap::TransformComponentInstance>();
		xform_comp.setScale(glm::vec3(window_size.x, window_size.y*-1.0f, 1.0f));
		xform_comp.setTranslate(glm::vec3(float(window_size.x) / 2.0f, float(window_size.y) / 2.0f, -900.0f));
	}
	
	

	void VinylApp::setCameraLocation()
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


	void VinylApp::setRecordColor()
	{
		// Fetch uniform associated with color of the record
		nap::EntityInstance* vinyl_entity = mModelEntity->getChildren()[0];
		RenderableMeshComponentInstance& render_mesh = vinyl_entity->getComponent<RenderableMeshComponentInstance>();
		nap::UniformVec3& record_clr_uniform = render_mesh.getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("recordColor");

		// Convert color to glm::vec3
		glm::vec3 new_color;
		new_color.x = mRecordColor.getRed();
		new_color.y = mRecordColor.getGreen();
		new_color.z = mRecordColor.getBlue();

		// Set
		record_clr_uniform.setValue(new_color);
	}
}