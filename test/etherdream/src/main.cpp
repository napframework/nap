// firstSDLapp.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "renderablemeshcomponent.h"
#include "laseroutputcomponent.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/ext.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/noise.hpp>

// Mod nap render includes
#include <renderservice.h>
#include <renderwindow.h>
#include <transformcomponent.h>
#include <orthocameracomponent.h>
#include <rendertarget.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <etherdreamservice.h>
#include <oscservice.h>
#include <sdlinput.h>
#include <sdlwindow.h>
#include <inputservice.h>
#include <inputcomponent.h>
#include <mousebutton.h>
#include <sceneservice.h>
#include <nap/logger.h>
#include <etherdreamdac.h>
#include <perspcameracomponent.h>
#include <mathutils.h>
#include <oscsender.h>
#include <renderablemeshcomponent.h>
#include "lineselectioncomponent.h"

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::InputService* inputService = nullptr;
nap::SceneService* sceneService = nullptr;
nap::EtherDreamService* laserService = nullptr;
nap::OSCService* oscService = nullptr;

// Holds all render windows
nap::ObjectPtr<nap::RenderWindow> renderWindow = nullptr;

// Laser DAC
nap::ObjectPtr<nap::EntityInstance> laserPrototype = nullptr;

// Holds the osc sender
nap::ObjectPtr<nap::OSCSender> oscSender = nullptr;

//////////////////////////////////////////////////////////////////////////

// Some utilities
void runGame(nap::Core& core);	

// Called when the window is updating
void onUpdate()
{
	// If any changes are detected, and we are reloading, we need to do this on the correct context
	renderService->getPrimaryWindow().makeCurrent();
	resourceManagerService->checkForFileChanges();

	// Process events for all windows
	renderService->processEvents();

	// Process all events for osc
	oscService->update();

	// Update all resources
	resourceManagerService->update();

	// Update the scene
	sceneService->update();

	// Send an osc message
	nap::OSCEventPtr new_event = std::make_unique<nap::OSCEvent>("/color/1");
	new_event->addValue<float>(1.0f);
	//oscSender->send(*new_event);
}


// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources({ renderWindow });
	
	// Activate current window for drawing
	renderWindow->makeActive();

	// Clear back-buffer
	opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
	backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);

	nap::EntityInstance* spline_entity = laserPrototype->getChildren()[0];
	nap::EntityInstance* laser_output_entity = laserPrototype->getChildren()[1];
	nap::EntityInstance* camera_entity = laserPrototype->getChildren()[2];

	// Render spline
	nap::RenderableMeshComponentInstance& line_mesh = spline_entity->getComponent<nap::RenderableMeshComponentInstance>();
	renderService->renderObjects(backbuffer, camera_entity->getComponent<nap::PerspCameraComponentInstance>());

	// Swap back buffer
	renderWindow->swap();

	// Set the laser line to render
	nap::RenderableMeshComponentInstance& line = spline_entity ->getComponent<nap::RenderableMeshComponentInstance>();
	nap::TransformComponentInstance& xform = spline_entity->getComponent<nap::TransformComponentInstance>();

	std::vector<nap::LaserOutputComponentInstance*> outputs;
	laser_output_entity->getComponentsOfType<nap::LaserOutputComponentInstance>(outputs);
	assert(line.getMesh().get_type().is_derived_from(RTTI_OF(nap::PolyLine)));
	
	nap::PolyLine& poly_line = static_cast<nap::PolyLine&>(line.getMesh());
	for (const auto& output : outputs)
	{
		output->setLine(poly_line, xform.getGlobalTransform());
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


	// Collects all the errors
	nap::utility::ErrorState errorState;

	// Create input service
	inputService = core.getOrCreateService<nap::InputService>();

	// Create scene service
	sceneService = core.getOrCreateService<nap::SceneService>();

	// Create etherdream service
	laserService = core.getOrCreateService<nap::EtherDreamService>();
	if (!laserService->init(errorState))
	{
		nap::Logger::fatal("unable to create laser service: %s", errorState.toString().c_str());
		return false;
	}

	// Create osc service
	oscService = core.getOrCreateService<nap::OSCService>();
	if (!oscService->init(errorState))
	{
		nap::Logger::fatal("unable to create osc service: %s", errorState.toString().c_str());
		return false;
	}

	// Load scene
	if (!resourceManagerService->loadFile("data/etherdream/etherdream.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;        
	}


	// Store all render windows
	renderWindow = resourceManagerService->findObject<nap::RenderWindow>("Window");

	// Store laser dacs
	laserPrototype = resourceManagerService->findEntity("LaserPrototypeEntity");

	// Store sender
	oscSender = resourceManagerService->findObject<nap::OSCSender>("OscSender");

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
			// Check if we're dealing with an input event
			if (nap::isInputEvent(event))
			{
				// Fetch event
				nap::InputEventPtr input_event = nap::translateInputEvent(event);

				// If we pressed escape, quit the loop
				if (input_event->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
				{
					nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(input_event.get());

					if (press_event->mKey == nap::EKeyCode::KEY_f)
					{
						static bool fullscreen = true;
						resourceManagerService->findObject<nap::RenderWindow>("Window")->getWindow()->setFullScreen(fullscreen);
						fullscreen = !fullscreen;
					}
					else if (press_event->mKey == nap::EKeyCode::KEY_LEFT)
					{
						nap::EntityInstance* spline_entity = laserPrototype->getChildren()[0];
						nap::LineSelectionComponentInstance& line_selection = spline_entity->getComponent<nap::LineSelectionComponentInstance>();
						line_selection.setIndex(line_selection.getIndex() - 1);
					}
					else if (press_event->mKey == nap::EKeyCode::KEY_RIGHT)
					{
						nap::EntityInstance* spline_entity = laserPrototype->getChildren()[0];
						nap::LineSelectionComponentInstance& line_selection = spline_entity->getComponent<nap::LineSelectionComponentInstance>();
						line_selection.setIndex(line_selection.getIndex() + 1);
					}

					if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
						loop = false;
				}
				inputService->addEvent(std::move(input_event));
			}

			// Check if it's a window event
			else if (nap::isWindowEvent(event))
			{
				// Add input event for later processing
				renderService->addEvent(nap::translateWindowEvent(event));
			}
		}

		//////////////////////////////////////////////////////////////////////////


		// run update call
		onUpdate();

		// run render call
		onRender();
	}

	renderService->shutdown();
}
       
     
