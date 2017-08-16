// firstSDLapp.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "RenderableMeshComponent.h"
#include "lasershapes.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/ext.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>

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
#include <sdlinput.h>
#include <sdlwindow.h>
#include <inputservice.h>
#include <inputcomponent.h>
#include <mousebutton.h>
#include <sceneservice.h>
#include <nap/logger.h>
#include <etherdreamdac.h>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::InputService* inputService = nullptr;
nap::SceneService* sceneService = nullptr;
nap::EtherDreamService* laserService = nullptr;

// Holds all render windows
std::vector<nap::ObjectPtr<nap::RenderWindow>> renderWindows;

// Main camera
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;

// Laser DAC
nap::ObjectPtr<nap::EtherDreamDac> laser = nullptr;
nap::ObjectPtr<nap::EntityInstance> laserEntity = nullptr;

//////////////////////////////////////////////////////////////////////////
// LASER STUFF
//////////////////////////////////////////////////////////////////////////

/**
@brief Creates a simple drawable circle
**/

#define CIRCLE_POINTS								600
struct nap::EtherDreamPoint circle[CIRCLE_POINTS];


/**
@brief Calculate color value
**/
int16_t colorsin(float pos)
{
	int max_value = std::numeric_limits<int16_t>::max();
	int min_value = std::numeric_limits<int16_t>::min();

	// Get color value
	int res = (sin(pos) + 1) * max_value;
	res = res > max_value ? max_value : res;
	res = res < min_value ? min_value : res;
	return res;
}

void FillCircle(float phase, int mode) 
{
	int i;
	int max_value = std::numeric_limits<int16_t>::max();

	for (i = 0; i < CIRCLE_POINTS; i++) {
		struct nap::EtherDreamPoint *pt = &circle[i];
		float ip = (float)i * 2.0 * M_PI / (float)CIRCLE_POINTS;
		float ipf = fmod(ip + phase, 2.0 * M_PI);;

		switch (mode) {
		default:
		case 0: {
			float cmult = .05 * sin(30 * (ip - phase / 3));
			pt->X = sin(ip) * 20000 * (1 + cmult);
			pt->Y = cos(ip) * 20000 * (1 + cmult);
			break;
		}
		case 1: {
			float cmult = .10 * sin(10 * (ip - phase / 3));
			pt->X = sin(ip) * 20000 * (1 + cmult);
			pt->Y = cos(ip) * 20000 * (1 + cmult);
			break;
		}
		case 2: {
			ip *= 3;
			float R = 5;
			float r = 3;
			float D = 5;

			pt->X = 2500 * ((R - r)*cos(ip + phase) + D*cos((R - r)*ip / r));
			pt->Y = 2500 * ((R - r)*sin(ip + phase) - D*sin((R - r)*ip / r));
			break;
		}
		case 3: {
			int n = 5;
			float R = 5 * cos(M_PI / n) / cos(fmod(ip, (2 * M_PI / n)) - (M_PI / n));
			pt->X = 3500 * R*cos(ip + phase);
			pt->Y = 3500 * R*sin(ip + phase);
			break;
		}
		case 4: {
			float Xo = sin(ip);
			pt->X = 20000 * Xo * cos(phase / 4);
			pt->Y = 20000 * Xo * -sin(phase / 4);
			ipf = fmod(((Xo + 1) / 2.0) + phase / 3, 1.0) * 2 * M_PI;
		}
		}

		pt->R = colorsin(ipf);
		pt->G = colorsin(ipf + (2.0 * M_PI / 3.0));
		pt->B = colorsin(ipf + (4.0 * M_PI / 3.0));
		pt->I = max_value;
	}
}


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

	// Update all resources
	resourceManagerService->update();

	// Update the scene
	sceneService->update();

	
	static int phase = 0;
	int size = sizeof(nap::EtherDreamPoint) * CIRCLE_POINTS;

	// Fill the circle
	FillCircle(static_cast<float>(phase) / 50.0f, 2);
	
	//nap::LaserSquareComponentInstance& square_shape = laserEntity->getComponent<nap::LaserSquareComponentInstance>();

	// Send some data
	if (laser->getWriteStatus() == nap::EtherDreamInterface::EStatus::READY)
	{
		// Write circle
		bool write = laser->writeFrame(circle, CIRCLE_POINTS);
		phase++;
	}
}




// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources(renderWindows);
	
	// Activate current window for drawing
	renderWindows[0]->makeActive();

	// Clear back-buffer
	opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindows[0]->getWindow()->getBackbuffer());
	backbuffer.setClearColor(glm::vec4(0.0705f, 0.49f, 0.5647f, 1.0f));
	renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);

	renderWindows[0]->swap();
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
	laserService->init(errorState);

	// Load scene
	if (!resourceManagerService->loadFile("data/etherdream/etherdream.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;        
	} 
	
	// Get important entities
	cameraEntity = resourceManagerService->findEntity("CameraEntity");
	assert(cameraEntity != nullptr);

	// Store all render windows
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window"));

	// Store laser dacs
	laser = resourceManagerService->findObject<nap::EtherDreamDac>("Laser1");
	laserEntity = resourceManagerService->findEntity("LaserEntity1");

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

					if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
						loop = false;
				}
				inputService->addEvent(std::move(input_event));
			}

			// Check if it's a window event
			else if (nap::isWindowEvent(event))
			{
				// Add input event for later processing
				renderService->addEvent(std::move(nap::translateWindowEvent(event)));
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
       
     