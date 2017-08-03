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
#include <videoservice.h>
#include <video.h>
#include "RenderableMeshComponent.h"
#include "nap/logger.h"

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

static bool loopVideo = true;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::InputService* inputService = nullptr;
nap::SceneService* sceneService = nullptr;
nap::VideoService* videoService = nullptr;
std::vector<nap::ObjectPtr<nap::Video>> videoResources;

std::vector<nap::ObjectPtr<nap::RenderWindow>> renderWindows;
nap::ObjectPtr<nap::EntityInstance> cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance> videoEntity = nullptr;

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

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = elapsed_time - prev_elapsed_time;

	if (videoEntity != nullptr)
	{
		glm::vec2 window_size = renderWindows[0]->getWindow()->getSize();

		float new_window_height = -FLT_MAX;
		for (auto& video_resource : videoResources)
		{
			nap::utility::ErrorState error_state;
			if (!video_resource->update(delta_time, error_state))
			{
				nap::Logger::fatal(error_state.toString());
			}

			//std::cout << video_resource->getTimeStamp() << "\n";
			
			float aspect_ratio = (float)video_resource->getWidth() / (float)video_resource->getHeight();
			new_window_height = std::max(new_window_height, window_size.x / aspect_ratio);
		}

		window_size.y = new_window_height;		
		renderWindows[0]->getWindow()->setSize(window_size);

		nap::MaterialInstance& plane_material = videoEntity->getComponent<nap::RenderableMeshComponentInstance>().getMaterialInstance();
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("yTexture").setTexture(videoResources[0]->getYTexture());
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("uTexture").setTexture(videoResources[0]->getUTexture()); 
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("vTexture").setTexture(videoResources[0]->getVTexture());

		// We set the position/size of the root layout element to cover the full screen.
		nap::TransformComponentInstance& transform_component = videoEntity->getComponent<nap::TransformComponentInstance>();
		transform_component.setTranslate(glm::vec3(window_size.x*0.5, window_size.y*0.5, -1000.0f));
		transform_component.setScale(glm::vec3(window_size.x, window_size.y, 1.0));
	} 

	// Update the scene
	sceneService->update();

	prev_elapsed_time = elapsed_time;
}

// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources(renderWindows);

	{
		nap::RenderWindow* render_window = renderWindows[0].get();

		if (cameraEntity != nullptr)
		{
			render_window->makeActive();

			opengl::RenderTarget* render_target = (opengl::RenderTarget*)render_window->getWindow()->getBackbuffer();
			render_target->setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			renderService->clearRenderTarget(*render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

			renderService->renderObjects(*render_target, cameraEntity->getComponent<nap::OrthoCameraComponentInstance>());

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

	inputService = core.getOrCreateService<nap::InputService>();
	sceneService = core.getOrCreateService<nap::SceneService>();
	videoService = core.getOrCreateService<nap::VideoService>();

	nap::utility::ErrorState errorState;
	if (!videoService->init(errorState))
	{
		nap::Logger::fatal("Failed to init video service: \n %s", errorState.toString().c_str());
		return false;
	}

	if (!resourceManagerService->loadFile("data/videoplayer/videoplayer.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;        
	} 
	
	// Get important entities
	cameraEntity = resourceManagerService->findEntity("CameraEntity");
	assert(cameraEntity != nullptr);

	videoEntity = resourceManagerService->findEntity("VideoEntity");
	assert(videoEntity != nullptr);

	// Store all render windows
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window"));

	// Collect all video resources and play
	videoResources.push_back(resourceManagerService->findObject<nap::Video>("Video1"));
	for (auto& videoResource : videoResources)
	{
		videoResource->mLoop = true;
		videoResource->play();
	}

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
       
     