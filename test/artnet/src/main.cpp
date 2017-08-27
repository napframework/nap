#include <nap/configure.h>

// Mod nap render includes
#include <renderservice.h>
#include <renderwindow.h>
#include <transformcomponent.h>
#include <orthocameracomponent.h>
#include <rendertarget.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <sdlinput.h>
#include <sdlwindow.h>
#include <inputservice.h>
#include <inputcomponent.h>
#include <mousebutton.h>
#include <sceneservice.h>
#include <nap/logger.h>
#include <artnetservice.h>
#include <nap/event.h>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Services
nap::RenderService*				renderService = nullptr;
nap::ResourceManagerService*	resourceManagerService = nullptr;
nap::InputService*				inputService = nullptr;
nap::SceneService*				sceneService = nullptr;
nap::ArtnetService*				artnetService = nullptr;

// Holds all render windows
std::vector<nap::ObjectPtr<nap::RenderWindow>> renderWindows;

// Main camera
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;


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

	// Swap backbuffer
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

	// Create artnet service
	artnetService = core.getOrCreateService<nap::ArtnetService>();
	if (!artnetService->init(errorState))
	{
		nap::Logger::fatal(errorState.toString());
		return false;
	}

	// Load scene
	if (!resourceManagerService->loadFile("data/artnet/artnet.json", errorState))
	{
		nap::Logger::fatal("Unable to de-serialize resources: \n %s", errorState.toString().c_str());
		return false;        
	} 
	
	// Get important entities
	cameraEntity = resourceManagerService->findEntity("CameraEntity");
	assert(cameraEntity != nullptr);

	// Store all render windows
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window"));

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
       
     