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
#include <orthocameracomponent.h>
#include <mathutils.h>
#include <planemeshresource.h>
#include <spheremeshresource.h>
#include <rendertargetresource.h>
#include <slideshowcomponent.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/coreattributes.h>

// STD includes
#include <ctime>
#include "fractionlayoutcomponent.h"

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Window Name
std::string		programName			= "Model Loading Test";

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::Service* rpcService = nullptr;

std::vector<nap::ObjectPtr<nap::WindowResource>> renderWindows;
nap::ObjectPtr<nap::EntityInstance> slideShowEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> rootLayoutEntity = nullptr;

static float movementScale = 0.5f;
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
	// If any changes are detected, and we are reloading, we need to do this on the correct context
	renderService->getPrimaryWindow().makeCurrent();
	resourceManagerService->checkForFileChanges();

	if (cameraEntity == nullptr)
	{
		cameraEntity = resourceManagerService->findEntity("CameraEntity");
	}

	if (slideShowEntity == nullptr)
		slideShowEntity = resourceManagerService->findEntity("SlideShowEntity");

	if (rootLayoutEntity == nullptr)
		rootLayoutEntity = resourceManagerService->findEntity("RootEntity");


	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = elapsed_time - prev_elapsed_time;
	if (delta_time < 0.0001f)
	{
		delta_time = 0.0001f;
	}

	if (slideShowEntity != nullptr)
	{
		nap::SlideShowComponent& component = slideShowEntity->getComponent<nap::SlideShowComponent>();
		component.update(delta_time);
	}

	if (cameraEntity != nullptr)
		updateCamera(delta_time);

	glm::vec2 window_size = renderWindows[0]->getWindow()->getSize();

	if (rootLayoutEntity != nullptr)
	{
		// First layout element. We start at -1000.0f, a value in front of the camera that is 'far away' 
		nap::TransformComponent& transform_component = rootLayoutEntity->getComponent<nap::TransformComponent>();
		transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
		transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));

		nap::FractionLayoutComponent& layout = rootLayoutEntity->getComponent<nap::FractionLayoutComponent>();
		layout.updateLayout(window_size, glm::mat4(1.0f));
	}

	prev_elapsed_time = elapsed_time;
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
		nap::WindowResource* render_window = renderWindows[0].get();

		if (cameraEntity != nullptr)
		{
			render_window->makeActive();

			opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
			render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			renderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

			renderService->renderObjects(*render_target, cameraEntity->getComponent<nap::OrthoCameraComponent>());

			render_window->swap();
		}
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

	renderService->draw.signal.connect(renderSlot);
	renderService->update.signal.connect(updateSlot);

	//////////////////////////////////////////////////////////////////////////
	// Resources
	//////////////////////////////////////////////////////////////////////////

	nap::utility::ErrorState errorState;
	if (!resourceManagerService->loadFile("data/tommy/tommy.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;        
	}


	//////////////////////////////////////////////////////////////////////////


	renderWindows.push_back(resourceManagerService->findObject<nap::WindowResource>("Window"));

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

// 					for (nap::RenderWindowComponent* renderWindow : renderWindows)
// 						renderWindow->fullScreen.setValue(fullScreen);
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
				case SDLK_n:
				{
					if (slideShowEntity != nullptr)
					{
						nap::SlideShowComponent& component = slideShowEntity->getComponent<nap::SlideShowComponent>();
						component.cycleLeft();
					}
					break;
				}
				case SDLK_m:
				{
					if (slideShowEntity != nullptr)
					{
						nap::SlideShowComponent& component = slideShowEntity->getComponent<nap::SlideShowComponent>();
						component.cycleRight();
					}
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
				SDL_Window* native_window = SDL_GetWindowFromID(event.window.windowID);
				nap::WindowResource* window = renderService->findWindow(native_window);
				nap::OpenGLRenderWindow* opengl_window = (nap::OpenGLRenderWindow*)(window->getWindow());
				opengl_window->handleEvent(event);
			}
		}

		//////////////////////////////////////////////////////////////////////////

		// run render call
		renderService->render();
	}

	renderService->shutdown();
}
       
  