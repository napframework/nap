// firstSDLapp.cpp : Defines the entry point for the console application.
//

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/ext.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>

// Mod nap render includes
#include <renderservice.h>
#include <renderwindowresource.h>
#include <transformcomponent.h>
#include <orthocameracomponent.h>
#include <rendertargetresource.h>
#include "slideshowcomponent.h"
#include "fractionlayoutcomponent.h"

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <sdlinput.h>
#include <sdlwindow.h>
#include "uiinputrouter.h"
#include "inputservice.h"
#include "inputcomponent.h"
#include "mousebutton.h"


//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::InputService* inputService = nullptr;

std::vector<nap::ObjectPtr<nap::RenderWindowResource>> renderWindows;
nap::ObjectPtr<nap::EntityInstance> slideShowEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> rootLayoutEntity = nullptr;
nap::ObjectPtr<nap::UIInputRouter>	uiInputRouter = nullptr;

// Some utilities
void runGame(nap::Core& core);	

// Called when the window is updating
void onUpdate()
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

	if (cameraEntity != nullptr)
	{
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(&resourceManagerService->getRootEntity());

		uiInputRouter->setCamera(cameraEntity->getComponent<nap::OrthoCameraComponent>());
		inputService->handleInput(*renderWindows[0], *uiInputRouter, entities);
	}

	// Process events for all windows
	for (auto& window : renderWindows)
		window->processEvents();

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

	if (rootLayoutEntity != nullptr)
	{
		glm::vec2 window_size = renderWindows[0]->getWindow()->getSize();

		// First layout element. We start at -1000.0f, a value in front of the camera that is 'far away' 
		// We set the position/size of the root layout element to cover the full screen.
		nap::TransformComponent& transform_component = rootLayoutEntity->getComponent<nap::TransformComponent>();
		transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
		transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));

		// Update the layout
		nap::FractionLayoutComponent& layout = rootLayoutEntity->getComponent<nap::FractionLayoutComponent>();
		layout.updateLayout(window_size, glm::mat4(1.0f));
	}

	// Update the scene
	updateTransforms(resourceManagerService->getRootEntity());

	prev_elapsed_time = elapsed_time;
}

// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources(renderWindows);

	{
		nap::RenderWindowResource* render_window = renderWindows[0].get();

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

	renderService->draw.connect(std::bind(&onRender));
	renderService->update.connect(std::bind(&onUpdate));

	inputService = core.getOrCreateService<nap::InputService>();

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

	uiInputRouter = resourceManagerService->findObject<nap::UIInputRouter>("InputRouter");
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindowResource>("Window"));

	nap::ObjectPtr<nap::EntityInstance> buttonRightEntity = resourceManagerService->findEntity("ButtonRightEntity");
	nap::ObjectPtr<nap::EntityInstance> buttonLeftEntity = resourceManagerService->findEntity("ButtonLeftEntity");

	buttonRightEntity->getComponent<nap::PointerInputComponent>().pressed.connect([](const nap::PointerPressEvent& evt)
	{
		if (slideShowEntity != nullptr && evt.mButton == nap::EMouseButton::LEFT)
		{
			nap::SlideShowComponent& component = slideShowEntity->getComponent<nap::SlideShowComponent>();
			component.cycleRight();
		}
	});

	buttonLeftEntity->getComponent<nap::PointerInputComponent>().pressed.connect([](const nap::PointerPressEvent& evt)
	{
		if (slideShowEntity != nullptr && evt.mButton == nap::EMouseButton::LEFT)
		{
			nap::SlideShowComponent& component = slideShowEntity->getComponent<nap::SlideShowComponent>();
			component.cycleLeft();
		}
	});


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

			uint32_t window_id;
			nap::EventPtr translated_event = nap::translateInputEvent(event, window_id);
			if (translated_event == nullptr)
				translated_event = nap::translateWindowEvent(event, window_id);

			if (translated_event != nullptr)
			{
				SDL_Window* native_window = SDL_GetWindowFromID(window_id);
				nap::WindowResource* window = renderService->findWindow(native_window);
				window->addEvent(std::move(translated_event));
			}

			// Check if escape was pressed
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;

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
       
  