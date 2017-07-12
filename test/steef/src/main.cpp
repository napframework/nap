// main.cpp : Defines the entry point for the console application.
//

// Local Includes

// GLM
#include <glm/glm.hpp>

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderservice.h>
#include <renderwindowresource.h>
#include <transformcomponent.h>
#include <perspcameracomponent.h>
#include <mathutils.h>
#include <rendertargetresource.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <inputservice.h>
#include <nap/windowresource.h>
#include <nap/windowevent.h>
#include <nap/event.h>
#include <inputcomponent.h>
#include <inputrouter.h>
#include <nap/entityinstance.h>
#include <nap/componentinstance.h>
#include <sceneservice.h>


//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects

static nap::ObjectPtr<nap::ImageResource>				vinylLabelImg   = nullptr;
static nap::ObjectPtr<nap::ImageResource>				vinylCoverImg = nullptr;

nap::RenderService*										renderService = nullptr;
nap::ResourceManagerService*							resourceManagerService = nullptr;
nap::SceneService*										sceneService = nullptr;
nap::InputService*										inputService = nullptr;

nap::ObjectPtr<nap::RenderWindowResource>				renderWindow;
nap::ObjectPtr<nap::EntityInstance>						vinylEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						coverEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						modelEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						cameraEntity = nullptr;

nap::DefaultInputRouter inputRouter;


// Some utilities
void runGame(nap::Core& core);	

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

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = prev_elapsed_time - elapsed_time;
	delta_time = delta_time < 0.01f ? 0.01f : delta_time;

	// Get transform of vinyl record
	nap::TransformComponent*  model_xform = &modelEntity->getComponent<nap::TransformComponent>();
	nap::TransformComponent*  vinyl_xform = &vinylEntity->getComponent<nap::TransformComponent>();

	// Get rotation angle
	float rot_angle_radians_x = glm::radians(-90.0f);

	// Rotate vinyl
	vinyl_xform->setRotate(glm::rotate(glm::quat(), rot_angle_radians_x, glm::vec3(1.0, 0.0, 0.0)));

	float rot_speed_y = 0.125f;
	float rot_angle_y = elapsed_time * 360.0f * rot_speed_y;
	float rot_angle_radians_y = glm::radians(rot_angle_y);

	// Calculate rotation quaternion
	glm::quat rot_quat = glm::rotate(glm::quat(), (float)rot_angle_radians_y, glm::vec3(0.0, 1.0, 0.0));

	// Set rotation on vinyl component
	model_xform->setRotate(rot_quat);

	// Set some material values
	nap::MaterialInstance& vinyl_material = vinylEntity ->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();

	float v = (sin(elapsed_time) + 1.0f) / 2.0f;

	// Set uniforms
	glm::vec4 color(v, 1.0f-v, 1.0f, 1.0f);
	vinyl_material.getOrCreateUniform<nap::UniformVec4>("mColor").setValue(color);

	//////////////////////////////////////////////////////////////////////////
	// Camera Update
	//////////////////////////////////////////////////////////////////////////

	resourceManagerService->getRootEntity().update(delta_time);

	// Update the scene
	sceneService->update();
}


// Called when the window is going to render
void onRender()
{
	// Clear opengl context related resources that are not necessary any more
	renderService->destroyGLContextResources({ renderWindow });

	// Activate current window for drawing
	renderWindow->makeActive();

	// Render output texture to plane
	std::vector<nap::RenderableComponent*> components_to_render;
	components_to_render.push_back(&vinylEntity->getComponent<nap::RenderableMeshComponent>());
	components_to_render.push_back(&coverEntity->getComponent<nap::RenderableMeshComponent>());

	opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
	backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
	renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
	renderService->renderObjects(backbuffer, cameraEntity->getComponent<nap::PerspCameraComponent>(), components_to_render);

	renderWindow->swap();
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

	nap::Logger::info("initialized render service: %s", renderService->getName().c_str());

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
		return false;  
	}  

	// Extract loaded resources
	renderWindow = resourceManagerService->findObject<nap::RenderWindowResource>("Viewport");

	// Get vintl textures
	vinylLabelImg = resourceManagerService->findObject<nap::ImageResource>("LabelImage");
	vinylCoverImg = resourceManagerService->findObject<nap::ImageResource>("CoverImage");
	
	// Get entity that holds vinyl
	modelEntity = resourceManagerService->findEntity("ModelEntity");
	vinylEntity	= resourceManagerService->findEntity("VinylEntity");
	coverEntity = resourceManagerService->findEntity("CoverEntity");

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
       
 