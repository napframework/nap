// main.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "firstpersoncontroller.h"

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
#include <mathutils.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "rtti/rtticast.h"

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

static nap::ObjectPtr<nap::Image> testTexture = nullptr;
static nap::ObjectPtr<nap::Image> pigTexture = nullptr;
static nap::ObjectPtr<nap::Image> worldTexture = nullptr;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::SceneService* sceneService = nullptr;
nap::InputService* inputService = nullptr;

std::vector<nap::ObjectPtr<nap::RenderWindow>>			renderWindows;
nap::ObjectPtr<nap::RenderTarget>						textureRenderTarget;
nap::ObjectPtr<nap::EntityInstance>						pigEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						rotatingPlaneEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						planeEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						worldEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						orientationEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						cameraEntityLeft = nullptr;
nap::ObjectPtr<nap::EntityInstance>						cameraEntityRight = nullptr;
nap::ObjectPtr<nap::EntityInstance>						splitCameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>						defaultInputRouter = nullptr;

// Some utilities
void run(nap::Core& core);	

namespace py = pybind11;

// Called when the window is updating
void onUpdate()
{
	nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

	{
		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(cameraEntityLeft.get());

		nap::Window* window = renderWindows[0].get();
		inputService->processEvents(*window, input_router, entities);
	}

	{
		// Update input for second window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(cameraEntityRight.get());

		nap::Window* window = renderWindows[1].get();
		inputService->processEvents(*window, input_router, entities);
	}
	
	// Process events for all windows
	renderService->processEvents();

	// Need to make primary window active before reloading files, as renderer resources need to be created in that context
	renderService->getPrimaryWindow().makeCurrent();
	resourceManagerService->checkForFileChanges();

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = prev_elapsed_time - elapsed_time;
	if (delta_time < 0.01f)
	{
		delta_time = 0.01f;
	}

	resourceManagerService->getRootEntity().update(delta_time);

	// Retrieve source (resource) mesh data
	nap::IMesh& mesh = planeEntity->getComponent<nap::RenderableMeshComponentInstance>().getMesh();
	nap::Mesh* rtti_mesh = rtti_cast<nap::Mesh>(&mesh);
	assert(rtti_mesh != nullptr);
	const nap::Vec3VertexAttribute& src_position_attribute = rtti_mesh->GetAttribute<glm::vec3>(nap::MeshInstance::VertexAttributeIDs::GetPositionName());
	const std::vector<glm::vec3>& src_positions = src_position_attribute.getData();

	// Retrieve destination (instance) mesh data
	nap::MeshInstance& mesh_instance = mesh.getMeshInstance();
	nap::Vec3VertexAttribute& dst_position_attribute = mesh_instance.GetAttribute<glm::vec3>(nap::MeshInstance::VertexAttributeIDs::GetPositionName());
	std::vector<glm::vec3>& dst_positions = dst_position_attribute.getData();

	// Sine wave over our quad
	for (int index = 0; index != src_positions.size() - 1; ++index)
    {
        float s = sin(elapsed_time + (float)index * 0.2f);
        dst_positions[index] = src_positions[index] * glm::vec3(s,s,s);
    }

	dst_positions.back() = *dst_positions.begin();

	nap::utility::ErrorState errorState;
	if (!mesh_instance.update(errorState))
	{
		nap::Logger::fatal(errorState.toString());
	}

	// Update the scene
	sceneService->update();
}


// Called when the window is going to render
void onRender()
{
	renderService->destroyGLContextResources(renderWindows);


	// Render offscreen surface(s)
	{
		renderService->getPrimaryWindow().makeCurrent();

		// Render entire scene to texture
		renderService->clearRenderTarget(textureRenderTarget->getTarget());
		renderService->renderObjects(textureRenderTarget->getTarget(), cameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>());
	}


	// Render window 0
	{
		nap::RenderWindow* render_window = renderWindows[0].get();

		render_window->makeActive();

		// Render output texture to plane
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.push_back(&planeEntity->getComponent<nap::RenderableMeshComponentInstance>());
		components_to_render.push_back(&rotatingPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>());

		nap::MaterialInstance& plane_material = planeEntity->getComponent<nap::RenderableMeshComponentInstance>().getMaterialInstance();
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->getColorTexture());
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->getColorTexture());
		plane_material.getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		plane_material.getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		nap::MaterialInstance& rotating_plane_material = rotatingPlaneEntity->getComponent<nap::RenderableMeshComponentInstance>().getMaterialInstance();
		rotating_plane_material.getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->getColorTexture());
		rotating_plane_material.getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->getColorTexture());
		rotating_plane_material.getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		rotating_plane_material.getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		renderService->clearRenderTarget(backbuffer);
		renderService->renderObjects(backbuffer, cameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::PerspCameraComponentInstance>().setGridLocation(0, 0);
		components_to_render.clear();
		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponentInstance>());
		renderService->renderObjects(backbuffer, splitCameraEntity->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

		render_window->swap();
	}
	 
	// render window 1
	{
		nap::RenderWindow* render_window = renderWindows[1].get();

		render_window->makeActive();

		// Render specific object directly to backbuffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.push_back(&pigEntity->getComponent<nap::RenderableMeshComponentInstance>());

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, cameraEntityRight->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::PerspCameraComponentInstance>().setGridLocation(0, 1);
 		components_to_render.clear();
 		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponentInstance>());
 		renderService->renderObjects(backbuffer, splitCameraEntity->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

		render_window->swap(); 
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

	if (!resourceManagerService->loadFile("data/objects.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;
	}

	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window0"));
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window1"));

	pigTexture					= resourceManagerService->findObject<nap::Image>("PigTexture");
 	testTexture					= resourceManagerService->findObject<nap::Image>("TestTexture");
 	worldTexture				= resourceManagerService->findObject<nap::Image>("WorldTexture");
 	textureRenderTarget			= resourceManagerService->findObject<nap::RenderTarget>("PlaneRenderTarget");

	pigEntity					= resourceManagerService->findEntity("PigEntity");
	rotatingPlaneEntity			= resourceManagerService->findEntity("RotatingPlaneEntity");
	planeEntity					= resourceManagerService->findEntity("PlaneEntity");
	worldEntity					= resourceManagerService->findEntity("WorldEntity");
	orientationEntity			= resourceManagerService->findEntity("OrientationEntity");
	cameraEntityLeft			= resourceManagerService->findEntity("CameraEntityLeft");
	cameraEntityRight			= resourceManagerService->findEntity("CameraEntityRight");
	splitCameraEntity			= resourceManagerService->findEntity("SplitCameraEntity");	
	defaultInputRouter			= resourceManagerService->findEntity("DefaultInputRouterEntity");

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
	run(core);

	return 0;
}

void run(nap::Core& core)
{
	// Run function
	bool loop = true;

	// Loop
	while (loop)
	{
		opengl::Event event;
		if (opengl::pollEvent(event))
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
				}

				// Add event to input service for further processing
				inputService->addEvent(std::move(input_event));
			}

			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(event);
				if (window_event != nullptr)
					renderService->addEvent(std::move(window_event));
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
       
      
