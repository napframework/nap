// firstSDLapp.cpp : Defines the entry point for the console application.
//

// Local Includes
#include "objects.h"

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

// C++ Headers
#include <string>
#include <iostream>
#include <FreeImage.h>

// OpenGL / glew Headers
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// Mod nap render includes
#include <material.h>
#include <modelresource.h>
#include <imageresource.h>
#include <modelmeshcomponent.h>
#include <renderservice.h>
#include <renderwindowcomponent.h>
#include <openglrenderer.h>
#include <transformcomponent.h>
#include <cameracomponent.h>
#include <mathutils.h>
#include <planecomponent.h>
#include <spherecomponent.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>

// STD includes
#include <ctime>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Window Name
std::string		programName			= "Model Loading Test";
std::string		vertShaderName		= "shaders/shader.vert";
std::string		fragShaderName		= "shaders/shader.frag";
std::string		vertShaderNameTwo	= "shaders/shader_two.vert";
std::string		fragShaderNameTwo	= "shaders/shader_two.frag";

static const std::string testTextureName = "data/test.jpg";
static nap::ImageResource* testTexture = nullptr;
static const std::string pigTextureName = "data/pig_head.jpg";
static nap::ImageResource* pigTexture = nullptr;
static const std::string worldTextureName = "data/world_texture.jpg";
static nap::ImageResource* worldTexture = nullptr;
static float movementScale = 3.0f;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::Service* rpcService = nullptr;
nap::RenderWindowComponent* renderWindow = nullptr;
nap::CameraComponent* cameraComponent = nullptr;
nap::ModelMeshComponent* modelComponent = nullptr;
nap::PlaneComponent* planeComponent = nullptr;
nap::SphereComponent* sphereComponent = nullptr;
nap::ShaderResource* shaderResource = nullptr;
nap::ShaderResource* shaderResourceOne = nullptr;
nap::ShaderResource* shaderResourceTwo = nullptr;

// movement
bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;

// vertex Shader indices
nap::Entity* model  = nullptr;
nap::Entity* plane  = nullptr;
nap::Entity* sphere = nullptr;

// Window width / height on startup
unsigned int windowWidth(512);
unsigned int windowHeight(512);

// GLM
glm::mat4 modelMatrix;			// Store the model matrix

// Some utilities
void runGame(nap::Core& core);	
void updateCamera();

// Called when the window is updating
void onUpdate(const nap::SignalAttribute& signal)
{
	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();

	nap::TransformComponent* xform_v = modelComponent->getParent()->getComponent<nap::TransformComponent>();
	nap::TransformComponent* xform_p = planeComponent->getParent()->getComponent<nap::TransformComponent>();
	nap::TransformComponent* xform_s = sphereComponent->getParent()->getComponent<nap::TransformComponent>();

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
	xform_v->rotate.setValue(nap::quatToVector(rot_quat));

	// Set rotation on plane component
	xform_p->rotate.setValue(nap::quatToVector(rot_quat));
	xform_p->translate.setValue({ 1.5f, 0.0, 0.0f });
	xform_p->uniformScale.setValue(1.5f);

	// Set rotatation on sphere
	glm::quat rot_quat_sphere = glm::rotate(glm::quat(), (float)rot_angle_radians_sphere, glm::vec3(0.0, 1.0, 0.0));
	xform_s->rotate.setValue(nap::quatToVector(rot_quat_sphere));
	xform_s->translate.setValue({ 0.0f, 0.0f, -3.0f });
	xform_s->uniformScale.setValue(1.0f);

	// Set transform
	float xform_distance = 2.0f;
	float xform_speed = 1.0f;
	float xform_offset = sin(elapsed_time * xform_speed) * xform_distance;
	xform_v->translate.setValue({ -1.5f, 0.0f, 0.0f });

	// Set scale
	float scale_speed = 4.0f;
	float nscale = (sin(elapsed_time  * scale_speed) + 1) / 2.0f;
	nscale = nap::fit<float>(nscale, 0.0f, 1.0f, 0.25f, 1.0f);
	//xform_v->uniformScale.setValue(nscale);

	// Set some material values
	nap::Material* material = modelComponent->getMaterial();

	float v = (sin(elapsed_time) + 1.0f) / 2.0f;

	// Set uniforms
	glm::vec4 color(v, 1.0f-v, 1.0f, 1.0f);
	material->setUniformValue<glm::vec4>("mColor", color);
	material->setUniformValue<int>("mTextureIndex", static_cast<int>(0));

	// Bind correct texture and send to shader
	material->setUniformTexture("pigTexture", *pigTexture);
	material->setUniformTexture("testTexture", *testTexture);

	// Set plane uniforms
	planeComponent->getMaterial()->setUniformTexture("pigTexture", *testTexture);
	planeComponent->getMaterial()->setUniformTexture("testTexture", *testTexture);
	planeComponent->getMaterial()->setUniformValue<int>("mTextureIndex", 0);
	planeComponent->getMaterial()->setUniformValue<glm::vec4>("mColor", {1.0f, 1.0f, 1.0f, 1.0f});

	// Set sphere uniforms
	sphereComponent->getMaterial()->setUniformTexture("pigTexture", *worldTexture);
	sphereComponent->getMaterial()->setUniformTexture("testTexture", *worldTexture);
	sphereComponent->getMaterial()->setUniformValue<int>("mTextureIndex", 0);
	sphereComponent->getMaterial()->setUniformValue<glm::vec4>("mColor", { 1.0f, 1.0f, 1.0f, 1.0f });

	//////////////////////////////////////////////////////////////////////////
	// Camera Update
	//////////////////////////////////////////////////////////////////////////

	updateCamera();

	//nap::TransformComponent* cam_xform = cameraComponent->getParent()->getComponent<nap::TransformComponent>();
	//rot_quat = glm::rotate(glm::quat(), rot_angle_radians, glm::vec3(0.0, 1.0, 0.0));
	//cam_xform->rotate.setValue(nap::quatToVector(rot_quat));
}
NSLOT(updateSlot, const nap::SignalAttribute&, onUpdate)


void updateCamera()
{
	float elapsed_time = renderWindow->getDeltaTimeFloat();
	float movement = movementScale * elapsed_time;

	nap::TransformComponent* cam_xform = cameraComponent->getParent()->getComponent<nap::TransformComponent>();
	if (moveForward)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() + glm::vec3(0.0f, 0.0f, movement));
	}
	if (moveBackward)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() - glm::vec3(0.0f, 0.0f, movement));
	}
	if (moveLeft)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() - glm::vec3(movement, 0.0f, 0.0f));
	}
	if (moveRight)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() + glm::vec3(movement, 0.0f, 0.0f));
	}
}


// Called when the window is going to render
void onRender(const nap::SignalAttribute& signal)
{
	// Clear window
	opengl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
	opengl::clearDepth();
	opengl::clearStencil();

	// Render all objects
	std::vector<nap::RenderableComponent*> comps	{ modelComponent };
	//renderService->renderObjects(comps, *cameraComponent);
	renderService->renderObjects(*cameraComponent);
}
NSLOT(renderSlot, const nap::SignalAttribute&, onRender)


/**
* Initialize all the resources and instances used for drawing
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{
	core.initialize();

	//////////////////////////////////////////////////////////////////////////

	/*
	std::string rpcServiceTypename = "nap::JsonRpcService";
	RTTI::TypeInfo rpcServiceType = RTTI::TypeInfo::getByName(rpcServiceTypename);
	if (!rpcServiceType.isValid()) 
	{
		nap::Logger::fatal("Failed to retrieve type: '%s'", rpcServiceTypename.c_str());
		return -1;
	}
	
	rpcService = core.getOrCreateService(rpcServiceType);
	//rpcService->getAttribute<bool>("manual")->setValue(true);
	rpcService->getAttribute<bool>("running")->setValue(true);
	*/

	//////////////////////////////////////////////////////////////////////////
	// GL Service + Window
	//////////////////////////////////////////////////////////////////////////

	// Create render service
	renderService = core.getOrCreateService<nap::RenderService>();
	renderService->setRenderer(RTTI_OF(nap::OpenGLRenderer));
	nap::Logger::info("initialized render service: %s", renderService->getName().c_str());

	// Add window component
	nap::Entity& window_entity = core.addEntity("window");
	renderWindow = &window_entity.addComponent<nap::RenderWindowComponent>("main_window");
	renderWindow->size.setValue({ windowWidth, windowHeight });
	renderWindow->position.setValue({ (1920 / 2) - 256, 1080 / 2 - 256 });
	renderWindow->title.setValue("Wolla");
	renderWindow->sync.setValue(false);

	// Connect draw and update signals
	renderWindow->draw.signal.connect(renderSlot);
	renderWindow->update.signal.connect(updateSlot);

	//////////////////////////////////////////////////////////////////////////
	// Resources
	//////////////////////////////////////////////////////////////////////////

	// Create shader resource
	nap::ResourceManagerService* service = core.getOrCreateService<nap::ResourceManagerService>();
	service->setAssetRoot(".");

	// Load textures
	nap::Resource* pig_texture = service->getResource(pigTextureName);
	pigTexture = static_cast<nap::ImageResource*>(pig_texture);
	nap::Resource* tes_texture = service->getResource(testTextureName);
	testTexture = static_cast<nap::ImageResource*>(tes_texture);
	nap::Resource* world_texture = service->getResource(worldTextureName);
	worldTexture = static_cast<nap::ImageResource*>(world_texture);

	// Load first shader
	nap::Resource* shader_resource = service->getResource(fragShaderName);
	shaderResourceOne = static_cast<nap::ShaderResource*>(shader_resource);
	shaderResourceOne->load();
	
	// Load second shader
	nap::Resource* shader_resource_two = service->getResource(fragShaderNameTwo);
	shaderResourceTwo = static_cast<nap::ShaderResource*>(shader_resource_two);
	shaderResourceTwo->load();
	
	// Set resource
	shaderResource = shaderResourceOne;

	// Load model resource
	nap::Resource* model_resource = service->getResource("data/pig_head_alpha_rotated.fbx");
	if (model_resource == nullptr)
	{
		nap::Logger::warn("unable to load pig head model resource");
		return false;
	}
	nap::ModelResource* pig_model = static_cast<nap::ModelResource*>(model_resource);

	//////////////////////////////////////////////////////////////////////////
	// Entities
	//////////////////////////////////////////////////////////////////////////

	// Create model entity
	model = &(core.getRoot().addEntity("model"));
	nap::TransformComponent& tran_component = model->addComponent<nap::TransformComponent>();
	modelComponent = &model->addComponent<nap::ModelMeshComponent>("pig_head_mesh");

	// Create plane entity
	plane = &(core.getRoot().addEntity("plane"));
	nap::TransformComponent& plane_tran_component = plane->addComponent<nap::TransformComponent>();
	planeComponent = &plane->addComponent<nap::PlaneComponent>("draw_plane");

	// Create sphere entity
	sphere = &(core.getRoot().addEntity("sphere"));
	nap::TransformComponent& sphere_tran_component = sphere->addComponent<nap::TransformComponent>();
	sphereComponent = &sphere->addComponent<nap::SphereComponent>("draw_sphere");

	// Set same shader to be used by all mesh components
	nap::Material* material = modelComponent->getMaterial();
	assert(material != nullptr);
	material->shaderResourceLink.setResource(*shaderResource);

	nap::Material* plane_material = planeComponent->getMaterial();
	assert(plane_material != nullptr);
	plane_material->shaderResourceLink.setResource(*shaderResource);

	nap::Material* sphere_material = sphereComponent->getMaterial();
	assert(sphere_material != nullptr);
	sphere_material->shaderResourceLink.setResource(*shader_resource);

	// Link model resource
	modelComponent->modelResource.setResource(*pig_model);

	//////////////////////////////////////////////////////////////////////////
	// Extract Material Information
	//////////////////////////////////////////////////////////////////////////

	// Extract mesh
	const opengl::Mesh* mesh = pig_model->getMesh(0);
	if (mesh == nullptr)
	{
		nap::Logger::warn("unable to extract model mesh at index 0");
		return false;
	}

	// This tells what vertex buffer index belongs to what vertex shader input binding name
	material->linkVertexBuffer("in_Position", modelComponent->getMesh()->getVertexBufferIndex());
	material->linkVertexBuffer("in_Color", modelComponent->getMesh()->getColorBufferIndex());
	material->linkVertexBuffer("in_Uvs", modelComponent->getMesh()->getUvBufferIndex());

	// Do the same for the plane
	plane_material->linkVertexBuffer("in_Position", planeComponent->getMesh()->getVertexBufferIndex());
	plane_material->linkVertexBuffer("in_Color", planeComponent->getMesh()->getColorBufferIndex());
	plane_material->linkVertexBuffer("in_Uvs", planeComponent->getMesh()->getUvBufferIndex());

	// And sphere
	sphere_material->linkVertexBuffer("in_Position", sphereComponent->getMesh()->getVertexBufferIndex());
	sphere_material->linkVertexBuffer("in_Color", sphereComponent->getMesh()->getColorBufferIndex());
	sphere_material->linkVertexBuffer("in_Uvs", sphereComponent->getMesh()->getUvBufferIndex());

	//////////////////////////////////////////////////////////////////////////
	// Add Camera
	//////////////////////////////////////////////////////////////////////////
	nap::Entity& camera_entity = core.addEntity("camera");
	cameraComponent = &camera_entity.addComponent<nap::CameraComponent>();
	nap::TransformComponent& camera_transform = camera_entity.addComponent<nap::TransformComponent>();
	camera_transform.translate.setValue({ 0.0f, 0.0f, -5.0f });
	cameraComponent->clippingPlanes.setValue(glm::vec2(0.01, 1000.0f));

	// Set camera
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

	opengl::enableDepthTest(true);
	opengl::enableBlending(true);
	opengl::enableMultiSampling(true);
	opengl::setLineWidth(1.3f);
	opengl::setPointSize(2.0f);
	opengl::setPolygonMode(opengl::PolygonMode::FILL);

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
				case SDLK_1:
				{
					shaderResource = shaderResourceOne;
					modelComponent->getMaterial()->shaderResourceLink.setResource(*shaderResource);
					planeComponent->getMaterial()->shaderResourceLink.setResource(*shaderResource);
					break;
				}
				case SDLK_2:
				{
					shaderResource = shaderResourceTwo;
					modelComponent->getMaterial()->shaderResourceLink.setResource(*shaderResource);
					planeComponent->getMaterial()->shaderResourceLink.setResource(*shaderResource);
					break;
				}
				case SDLK_ESCAPE:
					loop = false;
					break;
				case SDLK_f:
				{
					static bool fullScreen = false;
					fullScreen = !fullScreen;
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
					renderWindow->size.setValue({ width, height });
					cameraComponent->setAspectRatio((float)width, (float)height);
					break;
				}
				case SDL_WINDOWEVENT_MOVED:
				{
					int x = event.window.data1;
					int y = event.window.data2;
					renderWindow->position.setValue({ x,y });
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
