// firstSDLapp.cpp : Defines the entry point for the console application.
//

// SDL
#include <SDL.h>

// Naivi GL
#include <nopengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <chrono>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

// C++ Headers
#include <string>
#include <iostream>
#include <FreeImage.h>

// OpenGL / glew Headers
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// Mod nap render includes
#include <material.h>
#include <meshresource.h>
#include <imageresource.h>
#include <renderablemeshcomponent.h>
#include <renderservice.h>
#include <renderwindowcomponent.h>
#include <openglrenderer.h>
#include <transformcomponent.h>
#include <cameracomponent.h>
#include <mathutils.h>
#include <planecomponent.h>
#include <spherecomponent.h>
#include <rendertargetresource.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/coreattributes.h>

// STD includes
#include <ctime>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Window Name
std::string		programName			= "Model Loading Test";

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::Service* rpcService = nullptr;
std::vector<nap::RenderWindowComponent*> renderWindows;

nap::CameraComponent* cameraComponent = nullptr;

// Window width / height on startup
unsigned int windowWidth(512);
unsigned int windowHeight(512);

// Some utilities
void runGame(nap::Core& core);	
void updateCamera(float elapsedTime);

// Called when the window is updating
void onUpdate(const nap::SignalAttribute& signal)
{
	renderWindows[0]->makeActive();	// TEMP: if any changes are detected, and we are reloading, we need to do this on the correct context
	resourceManagerService->checkForFileChanges();

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = prev_elapsed_time - elapsed_time;
	if (delta_time < 0.01f)
	{
		delta_time = 0.01f;
	}
}
nap::Slot<const nap::SignalAttribute&> updateSlot = { [](const nap::SignalAttribute& attr){ onUpdate(attr); } };


// Called when the window is going to render
void onRender(const nap::SignalAttribute& signal)
{
	renderService->destroyGLContextResources(renderWindows);

	{
		nap::RenderWindowComponent* render_window = renderWindows[0];

		render_window->makeActive();

		opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
		render_target->setClearColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		renderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR);
		render_window->swap();
	}
}
nap::Slot<const nap::SignalAttribute&> renderSlot = { [](const nap::SignalAttribute& attr){ onRender(attr); } };


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
	if (!renderService->init(RTTI_OF(nap::OpenGLRenderer), error))
	{
		nap::Logger::fatal(error.toString());
		return false;
	}

	// Create windows
	int num_windows = 1;
	for (int index = 0; index < num_windows; ++index)
	{
		char name[100];
		sprintf(name, "Window %d", index);

		nap::Entity& window_entity = core.addEntity(name);
		
		// Create the window component (but don't add it to the entity yet), so that we can set the construction settings
		nap::RenderWindowComponent* renderWindow = RTTI_OF(nap::RenderWindowComponent).create<nap::RenderWindowComponent>();

		// If this is not the first window, make it share its OpenGL context with the first window
		nap::RenderWindowSettings settings;
		if (index != 0)
			settings.sharedWindow = renderWindows[0]->getWindow();

		// Set the construction settings and add it to the entity
		renderWindow->setConstructionSettings(settings);
		renderWindow->setName(name);
		window_entity.addComponent(std::move(std::unique_ptr<nap::RenderWindowComponent>(renderWindow)));

		renderWindow->size.setValue({ windowWidth, windowHeight });
		renderWindow->position.setValue({ (1920 / 2) - 256, 1080 / 2 - 256 });
		renderWindow->title.setValue(name);
		renderWindow->sync.setValue(false);		

		renderWindows.push_back(renderWindow);
	}

	renderService->draw.signal.connect(renderSlot);
	renderService->update.signal.connect(updateSlot);

	//////////////////////////////////////////////////////////////////////////
	// Resources
	//////////////////////////////////////////////////////////////////////////

	// Make the first ("root") window active so that the resources are created for the right context
	renderWindows[0]->makeActive();

	nap::utility::ErrorState errorState;
	if (!resourceManagerService->loadFile("data/tommy.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;
	}

	// Set render states
	nap::RenderState& render_state = renderService->getRenderState();
	render_state.mEnableDepthTest = true;
	render_state.mEnableBlending = true;
	render_state.mEnableMultiSampling = true;
	render_state.mLineWidth = 1.3f;
	render_state.mPointSize = 2.0f;
	render_state.mPolygonMode = opengl::PolygonMode::FILL;

	//////////////////////////////////////////////////////////////////////////
	// Add Camera
	//////////////////////////////////////////////////////////////////////////

	// Normal camera
	nap::Entity& camera_entity = core.addEntity("camera");
	cameraComponent = &camera_entity.addComponent<nap::CameraComponent>();
	nap::TransformComponent& camera_transform = camera_entity.addComponent<nap::TransformComponent>();
	camera_transform.translate.setValue({ 0.0f, 0.0f, 5.0f });
	cameraComponent->clippingPlanes.setValue(glm::vec2(0.01f, 1000.0f));
	cameraComponent->fieldOfView.setValue(45.0f);
	cameraComponent->setAspectRatio((float)windowWidth, (float)windowHeight);

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
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			// Check if we need to quit
			if (event.type == SDL_QUIT)
				loop = false;

			// Check if escape was pressed
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;
				case SDLK_f:
				{
					static bool fullScreen = false;
					fullScreen = !fullScreen;

					for (nap::RenderWindowComponent* renderWindow : renderWindows)
						renderWindow->fullScreen.setValue(fullScreen);
					break;
				}
				default:
					break;
				}
			}

// 			if (event.type == SDL_KEYUP)
// 			{
// 				switch (event.key.keysym.sym)
// 				{
// 				default:
// 					break;
// 				}
// 			}

			if (event.type == SDL_WINDOWEVENT)
			{
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
				{
					int width = event.window.data1;
					int height = event.window.data2;

					SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);

					for (nap::RenderWindowComponent* renderWindow : renderWindows)
					{
						if (renderWindow->getWindow()->getWindow() == window)
							renderWindow->size.setValue({ width, height });
					}

					cameraComponent->setAspectRatio((float)width, (float)height);
					break;
				}
				case SDL_WINDOWEVENT_MOVED:
				{
					int x = event.window.data1;
					int y = event.window.data2;

					SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);

					for (nap::RenderWindowComponent* renderWindow : renderWindows)
					{
						if (renderWindow->getWindow()->getWindow() == window)
							renderWindow->position.setValue({ x,y });
					}
						
					break;
				}
				default:
					break;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////

		// run render call
		renderService->render();
	}

	renderService->shutdown();
}
       
