// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "rotatecomponent.h"

// GLM
#include <glm/glm.hpp>

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderservice.h>
#include <renderwindow.h>
#include <transformcomponent.h>
#include <perspcameracomponent.h>
#include <mathutils.h>
#include <rendertarget.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/resourcemanager.h>
#include <inputservice.h>
#include <nap/windowresource.h>
#include <nap/windowevent.h>
#include <nap/event.h>
#include <inputcomponent.h>
#include <inputrouter.h>
#include <nap/entity.h>
#include <nap/component.h>
#include <sceneservice.h>
#include <orthocameracomponent.h>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects

static nap::ObjectPtr<nap::Image>				vinylLabelImg   = nullptr;
static nap::ObjectPtr<nap::Image>				vinylCoverImg = nullptr;

nap::RenderService*										renderService = nullptr;
nap::ResourceManagerService*							resourceManagerService = nullptr;
nap::SceneService*										sceneService = nullptr;
nap::InputService*										inputService = nullptr;

nap::ObjectPtr<nap::RenderWindow>				renderWindow;
nap::ObjectPtr<nap::EntityInstance>						modelEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						backgroundEntity = nullptr;

nap::DefaultInputRouter inputRouter;


// predefines
void runGame(nap::Core& core);


/**
* updates the background image to match the size of the output window
*/
void updateBackgroundImage()
{
	// Get size
	glm::ivec2 window_size = renderWindow->getWindow()->getSize();

	// Now update background texture
	nap::TransformComponentInstance& xform_comp = backgroundEntity->getComponent<nap::TransformComponentInstance>();
	xform_comp.setScale(glm::vec3(window_size.x, window_size.y*-1.0f, 0.0f));
	xform_comp.setTranslate(glm::vec3(float(window_size.x) / 2.0f, float(window_size.y) / 2.0f, 0.0f));
}


void updateShader()
{
	nap::TransformComponentInstance& cam_xform = cameraEntity->getComponent<nap::TransformComponentInstance>();
	
	// Set camera location on shader, used for rendering highlights
	for (const nap::EntityInstance* e : modelEntity->getChildren())
	{
		nap::RenderableMeshComponentInstance* mesh = e->findComponent<nap::RenderableMeshComponentInstance>();
		if(mesh == nullptr)
			continue;

		nap::UniformVec3& cameraLocation = mesh->getMaterialInstance().getOrCreateUniform<nap::UniformVec3>("cameraLocation");
		cameraLocation.setValue(cam_xform.getTranslate());
	}
}


// Called when the window is updating
void onUpdate()
{
	// Update input for first window
	std::vector<nap::EntityInstance*> entities;
	entities.push_back(cameraEntity.get());
	inputService->processEvents(*renderWindow, inputRouter, entities);
	
	// Process events for all windows
	renderService->processEvents();

	// Need to make primary window active before reloading files, as renderer resources need to be created in that context
	renderService->getPrimaryWindow().makeCurrent();
	
	// Reload all files if data changed
	resourceManagerService->checkForFileChanges();

	// Tick for all components that are listening
	resourceManagerService->update();

	// Update the scene
	sceneService->update();

	// Make sure background image matches window size
	updateBackgroundImage();

	// Update our shader variables
	updateShader();

}


// Called when the window is going to render
void onRender()
{
	// Clear opengl context related resources that are not necessary any more
	renderService->destroyGLContextResources({ renderWindow });

	// Activate current window for drawing
	renderWindow->makeActive();

	// Clear back-buffer
	opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
	backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
	renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
	
	// Render Background
	std::vector<nap::RenderableComponentInstance*> components_to_render;
	components_to_render.emplace_back(&(backgroundEntity->getComponent<nap::RenderableMeshComponentInstance>()));
	renderService->renderObjects(backbuffer, cameraEntity->getComponent<nap::OrthoCameraComponentInstance>(), components_to_render);

	// Render Vinyl
	components_to_render.clear();
	for (const nap::EntityInstance* e : modelEntity->getChildren())
	{
		if (e->hasComponent<nap::RenderableMeshComponentInstance>())
			components_to_render.emplace_back(&(e->getComponent<nap::RenderableMeshComponentInstance>()));
	}
	renderService->renderObjects(backbuffer, cameraEntity->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

	// Update gpu frame
	renderWindow->swap();
}


/**
 * Handles the window event
 * When the window size changes we want to update the background texture to reflect those changes, ie:
 * Scale to the right size
 */
void handleWindowEvent(const nap::WindowEvent& windowEvent)
{
	nap::rtti::TypeInfo e_type = windowEvent.get_type();
	if (e_type.is_derived_from(RTTI_OF(nap::WindowResizedEvent)) ||
		e_type.is_derived_from(RTTI_OF(nap::WindowShownEvent)))
	{
		nap::Logger::debug("window resized");
		updateBackgroundImage();
	}
}

/**
* Initialize all the resources and instances used for drawing
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{
	core.initialize();

	//////////////////////////////////////////////////////////////////////////
	// GL Service + Window
	//////////////////////////////////////////////////////////////////////////

	// Get resource manager service
	resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();

	// Create render service
	renderService = core.getOrCreateService<nap::RenderService>();
	
	nap::utility::ErrorState error;
	if (!renderService->init(error))
	{
		nap::Logger::fatal(error.toString());
		return false;
	}
	nap::Logger::info("initialized render service: %s", renderService->getTypeName().c_str());

	//////////////////////////////////////////////////////////////////////////
	// Input
	//////////////////////////////////////////////////////////////////////////

	inputService = core.getOrCreateService<nap::InputService>();

	//////////////////////////////////////////////////////////////////////////
	// Scene
	//////////////////////////////////////////////////////////////////////////
	sceneService = core.getOrCreateService<nap::SceneService>();

	//////////////////////////////////////////////////////////////////////////
	// Resources
	//////////////////////////////////////////////////////////////////////////

	nap::utility::ErrorState errorState;
	if (!resourceManagerService->loadFile("data/steef/objects.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		
		assert(false);
		return false;  
	}

	// Extract loaded resources
	renderWindow = resourceManagerService->findObject<nap::RenderWindow>("Viewport");
	renderWindow->onWindowEvent.connect(std::bind(&handleWindowEvent, std::placeholders::_1));

	// Get vintl textures
	vinylLabelImg = resourceManagerService->findObject<nap::Image>("LabelImage");
	vinylCoverImg = resourceManagerService->findObject<nap::Image>("CoverImage");
	
	// Get entity that holds vinyl
	modelEntity = resourceManagerService->findEntity("ModelEntity");

	// Get entity that holds the background image
	backgroundEntity = resourceManagerService->findEntity("BackgroundEntity");

	// Get entity that holds the camera
	cameraEntity = resourceManagerService->findEntity("CameraEntity");

	// Set render states
	nap::RenderState& render_state = renderService->getRenderState();
	render_state.mEnableMultiSampling = true;
	render_state.mLineWidth = 1.3f;
	render_state.mPointSize = 2.0f;
	render_state.mPolygonMode = opengl::PolygonMode::FILL;

	return true;
}


// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Initialize render stuff
	if (!init(core))
		return -1;

	// Run Gam
	runGame(core);

	return 0;
}

void runGame(nap::Core& core)
{
	// Run function
	bool loop = true;


	// Loop
	while (loop)
	{
		opengl::Event event;
		if (opengl::pollEvent(event))
		{
			// Check if we are dealing with an input event (mouse / keyboard)
			if (nap::isInputEvent(event))
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);

				// If we pressed escape, quit the loop
				if (input_event->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
				{
					nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(input_event.get());
					if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
						loop = false;

					if (press_event->mKey == nap::EKeyCode::KEY_f)
					{
						static bool fullscreen = true;
						resourceManagerService->findObject<nap::RenderWindow>("Viewport")->getWindow()->setFullScreen(fullscreen);
						fullscreen = !fullscreen;
					}
				}

				// Add event to input service for further processing
				inputService->addEvent(std::move(input_event));
			}

			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr nap_event = nap::translateWindowEvent(event);
				renderService->addEvent(std::move(nap_event));
			}
		}

		//////////////////////////////////////////////////////////////////////////

		// Update
		onUpdate();

		// Render
		onRender();
	}
	renderService->shutdown();
}
       
 