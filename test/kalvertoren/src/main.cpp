// main.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "firstpersoncontroller.h"
#include "pointlightcomponent.h"

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
#include <ntexturerendertarget2d.h>
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
#include <nap/entity.h>
#include <nap/component.h>
#include <nap/logger.h>
#include <sceneservice.h>
#include <videoservice.h>
#include <video.h>
#include <texture2d.h>
#include <mathutils.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "rtti/rtticast.h"
#include "orbitcontroller.h"
#include "cameracomponent.h"
#include "cameracontroller.h"

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::RenderService*								renderService = nullptr;
nap::ResourceManagerService*					resourceManagerService = nullptr;
nap::SceneService*								sceneService = nullptr;
nap::VideoService*								videoService = nullptr;
nap::InputService*								inputService = nullptr;

nap::ObjectPtr<nap::RenderWindow>				renderWindow;
nap::ObjectPtr<nap::RenderTarget>				textureRenderTarget;
nap::ObjectPtr<nap::EntityInstance>				kalvertorenEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>				cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>				defaultInputRouter = nullptr;
nap::ObjectPtr<nap::EntityInstance>				videoEntity = nullptr;
nap::ObjectPtr<nap::Video>						videoResource;
nap::ObjectPtr<nap::EntityInstance>				lightEntity = nullptr;
nap::ObjectPtr<nap::Material>					frameMaterial = nullptr;
nap::ObjectPtr<nap::Material>					vertexMaterial = nullptr;
		
std::vector<uint8_t> videoPlaybackData;

// Some utilities
void runGame(nap::Core& core);	

namespace py = pybind11;


// Updates the camera location for all the lights
void updateCameraLocation()
{
	// Set cam location
	const glm::mat4x4& cam_xform = cameraEntity->getComponent<nap::TransformComponentInstance>().getGlobalTransform();
	glm::vec3 cam_pos(cam_xform[3][0], cam_xform[3][1], cam_xform[3][2]);

	nap::UniformVec3& frame_cam_pos = frameMaterial->getUniform<nap::UniformVec3>("cameraLocation");
	nap::UniformVec3& verte_cam_pos = vertexMaterial->getUniform<nap::UniformVec3>("cameraLocation");
	frame_cam_pos.setValue(cam_pos);
	verte_cam_pos.setValue(cam_pos);

	// Set light location
	const glm::mat4x4 light_xform = lightEntity->getComponent<nap::TransformComponentInstance>().getGlobalTransform();
	glm::vec3 light_pos(light_xform[3][0], light_xform[3][1], light_xform[3][2]);

	nap::UniformVec3& frame_light_pos = frameMaterial->getUniform<nap::UniformVec3>("lightPosition");
	nap::UniformVec3& verte_light_pos = vertexMaterial->getUniform<nap::UniformVec3>("lightPosition");
	frame_light_pos.setValue(light_pos);
	verte_light_pos.setValue(light_pos);
}



// Called when the window is updating
void onUpdate()
{
	nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

	{
		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(cameraEntity.get());

		inputService->processEvents(*renderWindow, input_router, entities);
	}
	
	// Process events for all windows
	renderService->processEvents(); 

	// Need to make primary window active before reloading files, as renderer resources need to be created in that context
	renderService->getPrimaryWindow().makeCurrent();
	resourceManagerService->checkForFileChanges();

	// Update all resources
	resourceManagerService->update();

	// Update all entities
	sceneService->update();

	// Update cam location for lights
	updateCameraLocation();

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = prev_elapsed_time - elapsed_time;
	if (delta_time < 0.01f)
	{
		delta_time = 0.01f;
	}

	// If the video is not currently playing, start playing it again. This is needed for real time editing; 
	// if the video resource is modified it will not automatically play again (playback is started during init), causing the output to be black
	if (!videoResource->isPlaying())
		videoResource->play();

	// Set video to plane
	nap::utility::ErrorState error_state;
	if (!videoResource->update(delta_time, error_state))
	{
		nap::Logger::fatal(error_state.toString());
	}

	nap::MaterialInstance& plane_material = videoEntity->getComponent<nap::RenderableMeshComponentInstance>().getMaterialInstance();
	plane_material.getOrCreateUniform<nap::UniformTexture2D>("yTexture").setTexture(videoResource->getYTexture());
	plane_material.getOrCreateUniform<nap::UniformTexture2D>("uTexture").setTexture(videoResource->getUTexture());
	plane_material.getOrCreateUniform<nap::UniformTexture2D>("vTexture").setTexture(videoResource->getVTexture());


	// Update camera location for materials
	/*
	nap::RenderableMeshComponentInstance& renderableMeshComponent = kalvertorenEntity->getComponent<nap::RenderableMeshComponentInstance>();
	nap::MeshInstance& mesh = renderableMeshComponent.getMeshInstance();

	nap::VertexAttribute<glm::vec4>& color_attr = mesh.GetAttribute<glm::vec4>(nap::MeshInstance::VertexAttributeIDs::GetColorName(0));
	nap::VertexAttribute<glm::vec3>& uv_attr = mesh.GetAttribute<glm::vec3>(nap::MeshInstance::VertexAttributeIDs::GetUVName(0));

	glm::vec2 renderTargetSize = textureRenderTarget->getColorTexture().getSize();
	int stride = 4 * renderTargetSize.x;

	for (int index = 0; index < color_attr.getCount(); ++index)
	{
		glm::vec4& color = color_attr.mData[index];
		glm::vec3& uv = uv_attr.mData[index];

		float u = std::min(std::max(uv.x, 0.0f), 1.0f);
		float v = std::min(std::max(uv.y, 0.0f), 1.0f);

		int x_pos = (int)std::round(u * renderTargetSize.x);
		int y_pos = (int)std::round(v * renderTargetSize.y);

		uint8_t* pixel = videoPlaybackData.data() + y_pos * stride + x_pos * 4;

		color.x = pixel[0] / 255.0f;
		color.y = pixel[1] / 255.0f;
		color.z = pixel[2] / 255.0f;
		color.w = pixel[3] / 255.0f;
	}

	nap::utility::ErrorState errorState;
	if (!mesh.update(errorState))
	{
		nap::Logger::fatal("Failed to update texture: %s", errorState.toString());
	}
	*/
}


// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources(std::vector<nap::ObjectPtr<nap::RenderWindow>>({ renderWindow }));

	nap::CameraComponentInstance& cameraComponentInstance = cameraEntity->getComponent<nap::CameraControllerInstance>().getCameraComponent();

	// Render offscreen surface(s)
	{
		renderService->getPrimaryWindow().makeCurrent();

		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.push_back(&videoEntity->getComponent<nap::RenderableMeshComponentInstance>());

		opengl::TextureRenderTarget2D& render_target = textureRenderTarget->getTarget();
		render_target.setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		renderService->clearRenderTarget(render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);
		renderService->renderObjects(render_target, cameraComponentInstance, components_to_render);

		render_target.getColorTexture().getData(videoPlaybackData);
	}

	// Render window 0
	{
		renderWindow->makeActive();

		// Render output texture to plane
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		kalvertorenEntity->getComponentsOfType<nap::RenderableComponentInstance>(components_to_render);

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		renderService->clearRenderTarget(backbuffer);
		renderService->renderObjects(backbuffer, cameraComponentInstance, components_to_render);

		renderWindow->swap();
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

	videoService = core.getOrCreateService<nap::VideoService>();
	if (!videoService->init(errorState))
	{
		nap::Logger::fatal("Failed to init video service: \n %s", errorState.toString().c_str());
		return false;
	}

	if (!resourceManagerService->loadFile("data/kalvertoren/kalvertoren.json", errorState))
	{ 
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;        
	}  

	glFlush();
	 
	renderWindow			= resourceManagerService->findObject<nap::RenderWindow>("Window0");
 	textureRenderTarget		= resourceManagerService->findObject<nap::RenderTarget>("PlaneRenderTarget");
	kalvertorenEntity		= resourceManagerService->findEntity("KalvertorenEntity");
	cameraEntity			= resourceManagerService->findEntity("CameraEntity");
	defaultInputRouter		= resourceManagerService->findEntity("DefaultInputRouterEntity");
	videoEntity				= resourceManagerService->findEntity("VideoEntity"); 
	lightEntity				= resourceManagerService->findEntity("LightEntity");
	vertexMaterial			= resourceManagerService->findObject<nap::Material>("VertexColorMaterial");
	frameMaterial			= resourceManagerService->findObject<nap::Material>("FrameMaterial");

	assert(videoEntity != nullptr);

	// Collect all video resources and play
	videoResource = resourceManagerService->findObject<nap::Video>("Video1");
	videoResource->play();

	// Set render states
	nap::RenderState& render_state = renderService->getRenderState();
	render_state.mEnableMultiSampling = true;
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

	// Run Game
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
		while (opengl::pollEvent(event))
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
						resourceManagerService->findObject<nap::RenderWindow>("Window0")->getWindow()->setFullScreen(fullscreen);
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
       
       
            