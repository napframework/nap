// main.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "objects.h"
#include "firstpersoncontroller.h"

// GLM
#include <glm/glm.hpp>

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderservice.h>
#include <renderwindowresource.h>
#include <transformcomponent.h>
#include <perspcameracomponent.h>
#include <mathutils.h>
#include <rendertargetresource.h>
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
#include <nap/entityinstance.h>
#include <nap/componentinstance.h>
#include <sceneservice.h>


//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

static nap::ObjectPtr<nap::ImageResource> testTexture = nullptr;
static nap::ObjectPtr<nap::ImageResource> pigTexture = nullptr;
static nap::ObjectPtr<nap::ImageResource> worldTexture = nullptr;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::SceneService* sceneService = nullptr;
nap::InputService* inputService = nullptr;

std::vector<nap::ObjectPtr<nap::RenderWindowResource>>	renderWindows;
nap::ObjectPtr<nap::TextureRenderTargetResource2D>		textureRenderTarget;
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
void runGame(nap::Core& core);	

// Called when the window is updating
void onUpdate()
{
	nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponent>().mInputRouter;

	{
		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(cameraEntityLeft.get());

		nap::WindowResource* window = renderWindows[0].get();
		inputService->handleInput(*window, input_router, entities);
	}

	{
		// Update input for second window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(cameraEntityRight.get());

		nap::WindowResource* window = renderWindows[1].get();
		inputService->handleInput(*window, input_router, entities);
	}
	
	// Process events for all windows
	for (auto& window : renderWindows)
		window->processEvents();

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

	nap::TransformComponent* pig_transform = &pigEntity->getComponent<nap::TransformComponent>();
	nap::TransformComponent* rotating_plane_transform = &rotatingPlaneEntity->getComponent<nap::TransformComponent>();
	nap::TransformComponent* world_sphere_transform = &worldEntity->getComponent<nap::TransformComponent>();

	// Get rotation angle
	float rot_speed = 0.1f;
	float rot_angle = elapsed_time * 360.0f * rot_speed;
	float rot_angle_radians = glm::radians(rot_angle);

	float rot_speed_sphere = 0.01f;
	float rot_angle_sphere = elapsed_time * 360.0f * rot_speed_sphere;
	float rot_angle_radians_sphere = glm::radians(rot_angle_sphere);

	// Calculate rotation quaternion
	glm::quat rot_quat = glm::rotate(glm::quat(), (float)rot_angle_radians, glm::vec3(0.0, 1.0, 0.0));

	// Set rotation on model component
	pig_transform->setRotate(rot_quat);

	// Set rotation on plane component
	rotating_plane_transform->setRotate(rot_quat);

	glm::quat quaternion;
	quaternion.w = 1.0f;

	// Set rotation on sphere
	glm::quat rot_quat_sphere = glm::rotate(glm::quat(), -1.0f*(float)rot_angle_radians_sphere, glm::vec3(0.0, 1.0, 0.0));
	world_sphere_transform->setRotate(rot_quat_sphere);
	world_sphere_transform->setTranslate({ glm::sin(elapsed_time) * 5.0f, 0.0f, -3.0f });

	// Set scale
	float scale_speed = 4.0f;
	float nscale = (sin(elapsed_time  * scale_speed) + 1) / 2.0f;
	nscale = nap::fit<float>(nscale, 0.0f, 1.0f, 0.25f, 1.0f);
	//xform_v->uniformScale.setValue(nscale);

	// Set some material values
	nap::MaterialInstance& material_instance = pigEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();

	float v = (sin(elapsed_time) + 1.0f) / 2.0f;

	// Set uniforms
	glm::vec4 color(v, 1.0f-v, 1.0f, 1.0f);
	material_instance.getOrCreateUniform<nap::UniformVec4>("mColor").setValue(color);

	// Set plane uniforms
	nap::MaterialInstance& plane_material = planeEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
	plane_material.getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*testTexture);
	plane_material.getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(*testTexture);
	plane_material.getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
	plane_material.getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

	nap::MaterialInstance& rotating_plane_material = rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
	rotating_plane_material.getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*testTexture);
	rotating_plane_material.getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(*testTexture);
	rotating_plane_material.getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
	rotating_plane_material.getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

	//////////////////////////////////////////////////////////////////////////
	// Camera Update
	//////////////////////////////////////////////////////////////////////////

	resourceManagerService->getRootEntity().update(delta_time);

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
		renderService->clearRenderTarget(textureRenderTarget->getTarget(), opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		renderService->renderObjects(textureRenderTarget->getTarget(), cameraEntityLeft->getComponent<nap::PerspCameraComponent>());
	}

	// Render window 0
	{
		nap::RenderWindowResource* render_window = renderWindows[0].get();

		render_window->makeActive();

		// Render output texture to plane
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(&planeEntity->getComponent<nap::RenderableMeshComponent>());
		components_to_render.push_back(&rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>());

		nap::MaterialInstance& plane_material = planeEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
		plane_material.getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material.getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material.getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		plane_material.getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		nap::MaterialInstance& rotating_plane_material = rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
		rotating_plane_material.getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material.getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material.getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		rotating_plane_material.getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, cameraEntityLeft->getComponent<nap::PerspCameraComponent>(), components_to_render);

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::PerspCameraComponent>().setGridLocation(0, 0);
		components_to_render.clear();
		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponent>());
		renderService->renderObjects(backbuffer, splitCameraEntity->getComponent<nap::PerspCameraComponent>(), components_to_render);

		render_window->swap();
	}
	 
	// render window 1
	{
		nap::RenderWindowResource* render_window = renderWindows[1].get();

		render_window->makeActive();

		// Render specific object directly to backbuffer
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(&pigEntity->getComponent<nap::RenderableMeshComponent>());

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, cameraEntityRight->getComponent<nap::PerspCameraComponent>(), components_to_render);

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::PerspCameraComponent>().setGridLocation(0, 1);
 		components_to_render.clear();
 		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponent>());
 		renderService->renderObjects(backbuffer, splitCameraEntity->getComponent<nap::PerspCameraComponent>(), components_to_render);

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

	nap::Logger::info("initialized render service: %s", renderService->getName().c_str());

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

	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindowResource>("Window0"));
	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindowResource>("Window1"));

	pigTexture					= resourceManagerService->findObject<nap::ImageResource>("PigTexture");
 	testTexture					= resourceManagerService->findObject<nap::ImageResource>("TestTexture");
 	worldTexture				= resourceManagerService->findObject<nap::ImageResource>("WorldTexture");
 	textureRenderTarget			= resourceManagerService->findObject<nap::TextureRenderTargetResource2D>("PlaneRenderTarget");

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
			// Check if we are dealing with an input event (mouse / keyboard)
			nap::EventPtr nap_event = nullptr;
			if (nap::isInputEvent(event))
			{
				nap_event = nap::translateInputEvent(event);

				// If we pressed escape, quit the loop
				if (nap_event->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
				{
					nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(nap_event.get());
					if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
						loop = false;
				}
			}

			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(event))
			{
				nap_event = nap::translateWindowEvent(event);
			}

			// Don't perform any actions if no valid event was mapped
			if (nap_event == nullptr)
			{
				continue;
			}

			// Get window resource
			nap::ObjectPtr<nap::WindowResource> window = renderService->getWindow(event.window.windowID);
			window->addEvent(std::move(nap_event));
		}

		//////////////////////////////////////////////////////////////////////////

		// Update
		onUpdate();

		// Render
		onRender();
	}

	renderService->shutdown();
}
       
 