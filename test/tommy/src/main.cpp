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
#include <planemeshresource.h>
#include <spheremeshresource.h>
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

nap::ObjectPtr<nap::EntityInstance> backgroundImageEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;

static float movementScale = 3.0f;
static float rotateScale = 1.0f;
bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;
bool lookUp = false;
bool lookDown = false;
bool lookLeft = false;
bool lookRight = false;

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

	updateCamera(delta_time);
}
nap::Slot<const nap::SignalAttribute&> updateSlot = { [](const nap::SignalAttribute& attr){ onUpdate(attr); } };

void updateCamera(float deltaTime)
{
	float movement = movementScale * deltaTime;
	float rotate = rotateScale * deltaTime;
	float rotate_rad = rotate;

	nap::TransformComponent* cam_xform = cameraEntity->findComponent<nap::TransformComponent>();

	//glm::vec3 lookat_pos = cam_xform->getGlobalTransform()[0];
	//glm::vec3 dir = glm::cross(glm::normalize(lookat_pos), glm::vec3(cam_xform->getGlobalTransform()[1]));
	//glm::vec3 dir_f = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0,1.0,0.0));
	//glm::vec3 dir_s = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0, 0.0, 1.0));
	//dir_f *= movement;
	//dir_s *= movement;

	glm::vec3 side(1.0, 0.0, 0.0);
	glm::vec3 forward(0.0, 0.0, 1.0);

	glm::vec3 dir_forward = glm::rotate(cam_xform->getRotate(), forward);
	glm::vec3 movement_forward = dir_forward * movement;

	glm::vec3 dir_sideways = glm::rotate(cam_xform->getRotate(), side);
	glm::vec3 movement_sideways = dir_sideways * movement;

	//nap::Logger::info("direction: %f, %f,%f", dir_f.x, dir_f.y, dir_f.z);

	if (moveForward)
	{
		cam_xform->setTranslate(cam_xform->getTranslate() - movement_forward);
	}
	if (moveBackward)
	{
		cam_xform->setTranslate(cam_xform->getTranslate() + movement_forward);
	}
	if (moveLeft)
	{
		cam_xform->setTranslate(cam_xform->getTranslate() - movement_sideways);
	}
	if (moveRight)
	{
		cam_xform->setTranslate(cam_xform->getTranslate() + movement_sideways);
	}
	if (lookUp)
	{
		glm::quat r = cam_xform->getRotate();
		glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(1.0, 0.0, 0.0));
		cam_xform->setRotate(nr);
	}
	if (lookDown)
	{
		glm::quat r = cam_xform->getRotate();
		glm::quat nr = glm::rotate(r, -1.0f * rotate_rad, glm::vec3(1.0, 0.0, 0.0));
		cam_xform->setRotate(nr);
	}
	if (lookRight)
	{
		glm::quat r = cam_xform->getRotate();
		glm::quat nr = glm::rotate(r, -1.0f*rotate_rad, glm::vec3(0.0, 1.0, 0.0));
		cam_xform->setRotate(nr);
	}
	if (lookLeft)
	{
		glm::quat r = cam_xform->getRotate();
		glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(0.0, 1.0, 0.0));
		cam_xform->setRotate(nr);
	}
}

// Called when the window is going to render
void onRender(const nap::SignalAttribute& signal)
{
	renderService->destroyGLContextResources(renderWindows);

	{
		nap::RenderWindowComponent* render_window = renderWindows[0];

		render_window->makeActive();

		opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
		render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		renderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

		std::vector<nap::RenderableComponent*> components_to_render;
	
		components_to_render.push_back(&backgroundImageEntity->getComponent<nap::RenderableMeshComponent>());
		renderService->renderObjects(*render_target, components_to_render, cameraEntity->getComponent<nap::CameraComponent>());

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
	if (!resourceManagerService->loadFile("data/tommy/tommy.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;       
	}

	//////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////


	// Set render states
	nap::RenderState& render_state = renderService->getRenderState();
	render_state.mEnableDepthTest = true;
	render_state.mEnableBlending = true;
	render_state.mEnableMultiSampling = true;
	render_state.mLineWidth = 1.3f;
	render_state.mPointSize = 2.0f;
	render_state.mPolygonMode = opengl::PolygonMode::FILL;

	cameraEntity = resourceManagerService->findEntity("CameraEntity");
	assert(cameraEntity != nullptr);

	cameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)windowWidth, (float)windowHeight);

	backgroundImageEntity = resourceManagerService->findEntity("BackgroundImageEntity");

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
				case SDLK_w:
				{
					moveForward = true;
					break;
				}
				case SDLK_s:
				{
					moveBackward = true;
					break;
				}
				case SDLK_a:
				{
					moveLeft = true;
					break;
				}
				case SDLK_d:
				{
					moveRight = true;
					break;
				}
				case SDLK_UP:
				{
					lookUp = true;
					break;
				}
				case SDLK_DOWN:
				{
					lookDown = true;
					break;
				}
				case SDLK_LEFT:
				{
					lookLeft = true;
					break;
				}
				case SDLK_RIGHT:
				{
					lookRight = true;
					break;
				}
				default:
					break;
				}
			}

			if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
				{
					moveForward = false;
					break;
				}
				case SDLK_s:
				{
					moveBackward = false;
					break;
				}
				case SDLK_a:
				{
					moveLeft = false;
					break;
				}
				case SDLK_d:
				{
					moveRight = false;
					break;
				}
				case SDLK_UP:
				{
					lookUp = false;
					break;
				}
				case SDLK_DOWN:
				{
					lookDown = false;
					break;
				}
				case SDLK_LEFT:
				{
					lookLeft = false;
					break;
				}
				case SDLK_RIGHT:
				{
					lookRight = false;
					break;
				}
				default:
					break;
				}
			}

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

					cameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)width, (float)height);
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
       
 