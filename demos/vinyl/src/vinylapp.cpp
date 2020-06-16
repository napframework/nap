#include "vinylapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <renderwindow.h>
#include <scene.h>
#include <imgui/imgui.h>
#include <uniforminstances.h>

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
		if (!mResourceManager->loadFile("vinyl.json", error))
			return false;
        
		// Extract loaded resources and listen to window resize events
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Viewport");
		mRenderWindow->mWindowEvent.connect(std::bind(&VinylApp::handleWindowEvent, this, std::placeholders::_1));

		// Fetch vinyl textures
		mVinylLabelImg = mResourceManager->findObject<nap::ImageFromFile>("LabelImage");
		mVinylCoverImg = mResourceManager->findObject<nap::ImageFromFile>("CoverImage");
		
		// Fetch scene
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Get entity that holds vinyl
		mModelEntity = scene->findEntity("ModelEntity");
		
		// Get entity that holds the background image
		mBackgroundEntity = scene->findEntity("BackgroundEntity");
		
		// Get entity that holds the camera
		mCameraEntity = scene->findEntity("CameraEntity");

		// Select window to draw GUI to
		mGuiService->selectWindow(mRenderWindow);

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
		// Set back buffer color
		mRenderWindow->setClearColor({ 0.0705f, 0.49f, 0.5647f, 1.0f });
		
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin the render pass
			mRenderWindow->beginRendering();

			// Render Background
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			components_to_render.emplace_back(&(mBackgroundEntity->getComponent<nap::RenderableMeshComponentInstance>()));
			mRenderService->renderObjects(*mRenderWindow, mCameraEntity->getComponent<nap::OrthoCameraComponentInstance>(), components_to_render);

			// Render Vinyl
			components_to_render.clear();
			for (const nap::EntityInstance* e : mModelEntity->getChildren())
			{
				if (e->hasComponent<nap::RenderableMeshComponentInstance>())
					components_to_render.emplace_back(&(e->getComponent<nap::RenderableMeshComponentInstance>()));
			}
			mRenderService->renderObjects(*mRenderWindow, mCameraEntity->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

			// Tell the GUI to draw
			mGuiService->draw();

			// End the render pass
			mRenderWindow->endRendering();

			// End recording framebuffer
			mRenderService->endRecording();
		}

		// Signal end of frame capture operation
		mRenderService->endFrame();
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
				quit();

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

	
	int VinylApp::shutdown()
	{
		return 0;
	}
	
	
	/**
	 * updates the background image to match the size of the output window
	 */
	void VinylApp::positionBackground()
	{
		// Get size
		glm::ivec2 window_size = { mRenderWindow->getWidthPixels(), mRenderWindow->getHeightPixels()} ;
		
		// Now update background texture
		nap::TransformComponentInstance& xform_comp = mBackgroundEntity->getComponent<nap::TransformComponentInstance>();
		xform_comp.setScale(glm::vec3(window_size.x, window_size.y, 1.0f));
		xform_comp.setTranslate(glm::vec3(float(window_size.x) / 2.0f, float(window_size.y) / 2.0f, -900.0f));
	}
	
	

	void VinylApp::setCameraLocation()
	{
		// Set camera location in shader, should be present in all materials
		nap::TransformComponentInstance& cam_xform = mCameraEntity->getComponent<nap::TransformComponentInstance>();
		for (const nap::EntityInstance* e : mModelEntity->getChildren())
		{
			nap::RenderableMeshComponentInstance* mesh = e->findComponent<nap::RenderableMeshComponentInstance>();
			if(mesh == nullptr)
				continue;

			UniformStructInstance* frag_ubo = mesh->getMaterialInstance().getOrCreateUniform("UBO");
			nap::UniformVec3Instance* cameraLocation = frag_ubo->getOrCreateUniform<nap::UniformVec3Instance>("cameraLocation");
			cameraLocation->setValue(cam_xform.getTranslate());
		}
	}


	void VinylApp::setRecordColor()
	{
		// Fetch uniform associated with color of the record
		nap::EntityInstance* vinyl_entity = mModelEntity->getChildren()[0];
		RenderableMeshComponentInstance& render_mesh = vinyl_entity->getComponent<RenderableMeshComponentInstance>();
		
		// Set to current record color
		UniformStructInstance* frag_ubo = render_mesh.getMaterialInstance().getOrCreateUniform("UBO");
		UniformVec3Instance* record_clr_uniform = frag_ubo->getOrCreateUniform<nap::UniformVec3Instance>("recordColor");
		record_clr_uniform->setValue(mRecordColor.toVec3());
	}
}
