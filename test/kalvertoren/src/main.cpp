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
nap::ResourceManager*							resourceManager = nullptr;
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
void onUpdate(double deltaTime)
{
	nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

	// Update input for first window
	std::vector<nap::EntityInstance*> entities;
	entities.push_back(cameraEntity.get());
	inputService->processEvents(*renderWindow, input_router, entities);

	// Update cam location for lights
	updateCameraLocation();

	// If the video is not currently playing, start playing it again. This is needed for real time editing; 
	// if the video resource is modified it will not automatically play again (playback is started during init), causing the output to be black
	if (!videoResource->isPlaying())
		videoResource->play();

	// Set video to plane
	nap::utility::ErrorState error_state;
	if (!videoResource->update(nap::math::max<float>(deltaTime, 0.01f), error_state))
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
		renderService->clearRenderTarget(backbuffer);
		renderService->renderObjects(backbuffer, cameraComponentInstance, components_to_render);

		renderWindow->swap();
	}

}


/**
* Initialize all the resources and instances used for drawing
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core, nap::utility::ErrorState& error)
{
	// Create services
	renderService = core.getService<nap::RenderService>();
	inputService  = core.getService<nap::InputService>();
	sceneService  = core.getService<nap::SceneService>();
	videoService  = core.getService<nap::VideoService>();

	// Get resource manager and load data
	resourceManager = core.getResourceManager();
	if (!resourceManager->loadFile("data/kalvertoren/kalvertoren.json", error))
	{
		return false;
	}
	 
	renderWindow			= resourceManager->findObject<nap::RenderWindow>("Window0");
 	textureRenderTarget		= resourceManager->findObject<nap::RenderTarget>("PlaneRenderTarget");
	kalvertorenEntity		= resourceManager->findEntity("KalvertorenEntity");
	cameraEntity			= resourceManager->findEntity("CameraEntity");
	defaultInputRouter		= resourceManager->findEntity("DefaultInputRouterEntity");
	videoEntity				= resourceManager->findEntity("VideoEntity"); 
	lightEntity				= resourceManager->findEntity("LightEntity");
	vertexMaterial			= resourceManager->findObject<nap::Material>("VertexColorMaterial");
	frameMaterial			= resourceManager->findObject<nap::Material>("FrameMaterial");

	assert(videoEntity != nullptr);

	// Collect all video resources and play
	videoResource = resourceManager->findObject<nap::Video>("Video1");
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

	// Initialize engine -> loads all the modules
	nap::utility::ErrorState error;
	if (!core.initializeEngine(error))
	{
		nap::Logger::fatal("Unable to initialize engine: %s", error.toString().c_str());
		return -1;
	}

	// Initialize render stuff
	if (!init(core, error))
	{
		nap::Logger::fatal("Unable to initialize app: %s", error.toString().c_str());
		return -1;
	}

	// Run Game
	runGame(core);

	return 0;
}


void runGame(nap::Core& core)
{
	// Run function
	bool loop = true;

	// Pointer to function used inside update call by core
	std::function<void(double)> update_call = std::bind(&onUpdate, std::placeholders::_1);

	// Signal Start
	core.start();

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
						resourceManager->findObject<nap::RenderWindow>("Window0")->getWindow()->setFullScreen(fullscreen);
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
		core.update(update_call);

		// Render
		onRender();
	}

	core.shutdown();
}
       
       
            