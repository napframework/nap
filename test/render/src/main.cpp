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
#include <meshcomponent.h>
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
std::string		vertShaderName		= "shaders/shader.vert";
std::string		fragShaderName		= "shaders/shader.frag";
std::string		vertShaderNameTwo	= "shaders/shader_two.vert";
std::string		fragShaderNameTwo	= "shaders/shader_two.frag";
std::string		orientationVertShaderName = "shaders/orientation.vert";
std::string		orientationFragShaderName = "shaders/orientation.frag";

static const std::string testTextureName = "data/test.jpg";
static nap::ImageResource* testTexture = nullptr;
static const std::string pigTextureName = "data/pig_head.jpg";
static nap::ImageResource* pigTexture = nullptr;
static const std::string worldTextureName = "data/world_texture.jpg";
static nap::ImageResource* worldTexture = nullptr;
static float movementScale = 3.0f;
static float rotateScale = 1.0f;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::Service* rpcService = nullptr;
std::vector<nap::RenderWindowComponent*> renderWindows;
nap::TextureRenderTargetResource2D* textureRenderTarget;
nap::CameraComponent* cameraComponent = nullptr;
nap::CameraComponent* splitCameraComponent = nullptr;
nap::MeshComponent* pigMeshComponent = nullptr;
nap::PlaneComponent* planeComponent = nullptr;
nap::SphereComponent* sphereComponent = nullptr;
nap::ShaderResource* shaderResource = nullptr;
nap::ShaderResource* orientationShaderResource = nullptr;
nap::MeshResource* orientationMesh = nullptr;
nap::MeshResource* pigMesh = nullptr;
nap::MeshComponent* orientationMeshComponent = nullptr;
nap::Material* generalMaterial = nullptr;
nap::Material* planeMaterial = nullptr;
nap::Material* sphereMaterial = nullptr;
nap::Material* orientationMaterial = nullptr;

// movement
bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;
bool lookUp = false;
bool lookDown = false;
bool lookLeft = false;
bool lookRight = false;

// vertex Shader indices
nap::Entity* model  = nullptr;
nap::Entity* plane  = nullptr;
nap::Entity* sphere = nullptr;
nap::Entity* orientation = nullptr;

// Window width / height on startup
unsigned int windowWidth(512);
unsigned int windowHeight(512);

// GLM
glm::mat4 modelMatrix;			// Store the model matrix

// Some utilities
void runGame(nap::Core& core);	
void updateCamera(float elapsedTime);
void createSpheres(nap::Core& core, nap::Resource& shader);

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

	nap::TransformComponent* xform_v = pigMeshComponent->getParent()->getComponent<nap::TransformComponent>();
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
	xform_v->rotate.setValue(rot_quat);

	// Set rotation on plane component
	xform_p->rotate.setValue(rot_quat);
	xform_p->translate.setValue({ 1.5f, 0.0, 0.0f });
	xform_p->uniformScale.setValue(1.5f);

	glm::quat quaternion;
	quaternion.w = 1.0f;

	// Set rotatation on sphere
	glm::quat rot_quat_sphere = glm::rotate(glm::quat(), -1.0f*(float)rot_angle_radians_sphere, glm::vec3(0.0, 1.0, 0.0));
	xform_s->rotate.setValue(rot_quat_sphere);

	xform_s->translate.setValue({ glm::sin(elapsed_time) * 5.0f, 0.0f, -3.0f });
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
	nap::Material* material = pigMeshComponent->getMaterial();

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

	updateCamera(delta_time);

	//nap::TransformComponent* cam_xform = cameraComponent->getParent()->getComponent<nap::TransformComponent>();
	//rot_quat = glm::rotate(glm::quat(), rot_angle_radians, glm::vec3(0.0, 1.0, 0.0));
	//cam_xform->rotate.setValue(nap::quatToVector(rot_quat));
}
nap::Slot<const nap::SignalAttribute&> updateSlot = { [](const nap::SignalAttribute& attr){ onUpdate(attr); } };


void updateCamera(float deltaTime)
{
	float movement = movementScale * deltaTime;
	float rotate = rotateScale * deltaTime;
	float rotate_rad = rotate;

	nap::TransformComponent* cam_xform = cameraComponent->getParent()->getComponent<nap::TransformComponent>();
	//glm::vec3 lookat_pos = cam_xform->getGlobalTransform()[0];
	//glm::vec3 dir = glm::cross(glm::normalize(lookat_pos), glm::vec3(cam_xform->getGlobalTransform()[1]));
	//glm::vec3 dir_f = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0,1.0,0.0));
	//glm::vec3 dir_s = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0, 0.0, 1.0));
	//dir_f *= movement;
	//dir_s *= movement;

	glm::vec3 side(1.0, 0.0, 0.0);
	glm::vec3 forward(0.0, 0.0, 1.0);

	glm::vec3 dir_forward = glm::rotate(cam_xform->rotate.getValue(),forward);
	glm::vec3 movement_forward = dir_forward * movement;

	glm::vec3 dir_sideways = glm::rotate(cam_xform->rotate.getValue(), side);
	glm::vec3 movement_sideways = dir_sideways * movement;
		
	//nap::Logger::info("direction: %f, %f,%f", dir_f.x, dir_f.y, dir_f.z);

	if (moveForward)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() - movement_forward);
	}
	if (moveBackward)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() + movement_forward);
	}
	if (moveLeft)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() - movement_sideways);
	}
	if (moveRight)
	{
		cam_xform->translate.setValue(cam_xform->translate.getValue() + movement_sideways);
	}
	if (lookUp)
	{
		glm::quat r = cam_xform->rotate.getValue();
		glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(1.0, 0.0, 0.0));
		cam_xform->rotate.setValue(nr);
	}
	if (lookDown)
	{
		glm::quat r = cam_xform->rotate.getValue();
		glm::quat nr = glm::rotate(r, -1.0f * rotate_rad, glm::vec3(1.0, 0.0, 0.0));
		cam_xform->rotate.setValue(nr);
	}
	if (lookRight)
	{
		glm::quat r = cam_xform->rotate.getValue();
		glm::quat nr = glm::rotate(r, -1.0f*rotate_rad, glm::vec3(0.0, 1.0, 0.0));
		cam_xform->rotate.setValue(nr);
	}
	if (lookLeft)
	{
		glm::quat r = cam_xform->rotate.getValue();
		glm::quat nr = glm::rotate(r, rotate_rad, glm::vec3(0.0, 1.0, 0.0));
		cam_xform->rotate.setValue(nr);
	}
}


// Called when the window is going to render
void onRender(const nap::SignalAttribute& signal)
{
	// Render window 0
	{
		nap::RenderWindowComponent* render_window = renderWindows[0];

		render_window->makeActive();
		
		// Render entire scene to texture
		renderService->clearRenderTarget(textureRenderTarget->getTarget(), opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		renderService->renderObjects(textureRenderTarget->getTarget(), *cameraComponent);

		// Render output texture to plane
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(planeComponent);

		planeComponent->getMaterial()->setUniformTexture("testTexture", textureRenderTarget->GetColorTexture());
		planeComponent->getMaterial()->setUniformTexture("pigTexture", textureRenderTarget->GetColorTexture());
		planeComponent->getMaterial()->setUniformValue<int>("mTextureIndex", 0);
		planeComponent->getMaterial()->setUniformValue<glm::vec4>("mColor", { 1.0f, 1.0f, 1.0f, 1.0f });

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, components_to_render, *cameraComponent);

		// Render sphere using split camera with custom projection matrix
		splitCameraComponent->setGridLocation(0, 0);
		components_to_render.clear();
		components_to_render.push_back(sphereComponent);
		renderService->renderObjects(backbuffer, components_to_render, *splitCameraComponent);

		render_window->swap();
	}
	 
	// render window 1
	{
		nap::RenderWindowComponent* render_window = renderWindows[1];

		render_window->makeActive();

		// Render specific object directly to backbuffer
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(pigMeshComponent);

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, components_to_render, *cameraComponent);

		// Render sphere using split camera with custom projection matrix
		splitCameraComponent->setGridLocation(0, 1);
 		components_to_render.clear();
 		components_to_render.push_back(sphereComponent);
 		renderService->renderObjects(backbuffer, components_to_render, *splitCameraComponent);

		render_window->swap();

	}

	static float fps_time = 0.0f;
	fps_time += renderWindows[0]->getDeltaTimeFloat();
	if (fps_time > 0.99f)
	{
		nap::Logger::info("fps: %f", renderWindows[0]->getFps());
		fps_time = 0.0f;
	}
}
nap::Slot<const nap::SignalAttribute&> renderSlot = { [](const nap::SignalAttribute& attr){ onRender(attr); } };


bool initResources(nap::InitResult& initResult)
{
	pigTexture = resourceManagerService->createResource<nap::ImageResource>();
	pigTexture->mImagePath = pigTextureName;
	if (!pigTexture->init(initResult))
		return false;
	
	testTexture = resourceManagerService->createResource<nap::ImageResource>();
	testTexture->mImagePath = testTextureName;
	if (!testTexture->init(initResult))
		return false;

	worldTexture = resourceManagerService->createResource<nap::ImageResource>();
	worldTexture->mImagePath = worldTextureName;
	if (!worldTexture->init(initResult))
		return false;

	nap::MemoryTextureResource2D* color_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	color_texture->mSettings.width = 640;
	color_texture->mSettings.height = 480;
	color_texture->mSettings.internalFormat = GL_RGBA;
	color_texture->mSettings.format = GL_RGBA;
	color_texture->mSettings.type = GL_UNSIGNED_BYTE;
	if (!color_texture->init(initResult))
		return false;

	nap::MemoryTextureResource2D* depth_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	depth_texture->mSettings.width = 640;
	depth_texture->mSettings.height = 480;
	depth_texture->mSettings.internalFormat = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.format = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.type = GL_FLOAT;
	if (!depth_texture->init(initResult))
		return false;
	
	// Create frame buffer
	textureRenderTarget = resourceManagerService->createResource<nap::TextureRenderTargetResource2D>();
	textureRenderTarget->setColorTexture(*color_texture);
	textureRenderTarget->setDepthTexture(*depth_texture);
	textureRenderTarget->mClearColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	if (!textureRenderTarget->init(initResult))
		return false;

	// Load general shader
	shaderResource = resourceManagerService->createResource<nap::ShaderResource>();
	shaderResource->mVertPath = vertShaderName;
	shaderResource->mFragPath = fragShaderName;
	if (!shaderResource->init(initResult))
		return false;

	// Load orientation shader
	orientationShaderResource = resourceManagerService->createResource<nap::ShaderResource>();
	orientationShaderResource->mVertPath = orientationVertShaderName;
	orientationShaderResource->mFragPath = orientationFragShaderName;
	if (!orientationShaderResource->init(initResult))
		return false;

	// Load orientation resource
	orientationMesh = resourceManagerService->createResource<nap::MeshResource>();
	orientationMesh->mPath = "data/orientation.mesh";
	if (!orientationMesh->init(initResult))
		return false;

	// Load model resource
	pigMesh = resourceManagerService->createResource<nap::MeshResource>();
	pigMesh->mPath = "data/pig_head_alpha_rotated.mesh";
	if (!pigMesh->init(initResult))
		return false;

	generalMaterial = resourceManagerService->createResource<nap::Material>();
	generalMaterial->mShader = shaderResource;
	if (!generalMaterial->init(initResult))
		return false;

	planeMaterial = resourceManagerService->createResource<nap::Material>();
	planeMaterial->mShader = shaderResource;
	if (!planeMaterial->init(initResult))
		return false;

	sphereMaterial = resourceManagerService->createResource<nap::Material>();
	sphereMaterial->mShader = shaderResource;
	if (!sphereMaterial->init(initResult))
		return false;

	orientationMaterial = resourceManagerService->createResource<nap::Material>();
	orientationMaterial->mShader = orientationShaderResource;
	if (!orientationMaterial->init(initResult))
		return false;

	return true;
}

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

	// Create windows
	int num_windows = 2;
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

	resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();

	nap::InitResult initResult;
#if 1
	if (!resourceManagerService->loadFile("data/objects.json", initResult))
	{
		nap::Logger::fatal("Unable to deserialize resources: %s", initResult.mErrorString.c_str());
		return false;
	}

	pigTexture = resourceManagerService->findResource<nap::ImageResource>("PigTexture");
 	testTexture = resourceManagerService->findResource<nap::ImageResource>("TestTexture");
 	worldTexture = resourceManagerService->findResource<nap::ImageResource>("WorldTexture");
 	textureRenderTarget = resourceManagerService->findResource<nap::TextureRenderTargetResource2D>("PlaneRenderTarget");
	shaderResource = resourceManagerService->findResource<nap::ShaderResource>("GeneralShader");
	orientationShaderResource = resourceManagerService->findResource<nap::ShaderResource>("OrientationShader");
	orientationMesh = resourceManagerService->findResource<nap::MeshResource>("OrientationMesh");
	pigMesh = resourceManagerService->findResource<nap::MeshResource>("PigMesh");
	generalMaterial = resourceManagerService->findResource<nap::Material>("GeneralMaterial");
	planeMaterial = resourceManagerService->findResource<nap::Material>("PlaneMaterial");
	sphereMaterial = resourceManagerService->findResource<nap::Material>("SphereMaterial");
	orientationMaterial = resourceManagerService->findResource<nap::Material>("OrientationMaterial");
#else	
	if (!initResources(initResult))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", initResult.mErrorString.c_str());
		return false;
	}
#endif

	// Set render states
	nap::RenderState& render_state = renderService->getRenderState();
	render_state.mEnableDepthTest = true;
	render_state.mEnableBlending = true;
	render_state.mEnableMultiSampling = true;
	render_state.mLineWidth = 1.3f;
	render_state.mPointSize = 2.0f;
	render_state.mPolygonMode = opengl::PolygonMode::FILL;

	//////////////////////////////////////////////////////////////////////////
	// Entities
	//////////////////////////////////////////////////////////////////////////

	// Create model entity
	model = &(core.getRoot().addEntity("model"));
	nap::TransformComponent& tran_component = model->addComponent<nap::TransformComponent>();
	pigMeshComponent = &model->addComponent<nap::MeshComponent>("pig_head_mesh");

	// Create orientation entity
	orientation = &(core.getRoot().addEntity("orientation"));
	nap::TransformComponent& or_tran_component = orientation->addComponent<nap::TransformComponent>();
	or_tran_component.uniformScale.setValue(0.33f);
	orientationMeshComponent = &orientation->addComponent<nap::MeshComponent>("orientation gizmo");

	// Create plane entity
	plane = &(core.getRoot().addEntity("plane"));
	nap::TransformComponent& plane_tran_component = plane->addComponent<nap::TransformComponent>();
	planeComponent = &plane->addComponent<nap::PlaneComponent>("draw_plane");

	// Create sphere entity
	sphere = &(core.getRoot().addEntity("sphere"));
	nap::TransformComponent& sphere_tran_component = sphere->addComponent<nap::TransformComponent>();
	sphereComponent = &sphere->addComponent<nap::SphereComponent>("draw_sphere");

	pigMeshComponent->setMaterial(generalMaterial);
	planeComponent->setMaterial(planeMaterial);
	sphereComponent->setMaterial(sphereMaterial);
	orientationMeshComponent->setMaterial(orientationMaterial);

	// Link model resource
	pigMeshComponent->mMesh = pigMesh;

	// Link orientation resource
	orientationMeshComponent->mMesh = orientationMesh;

	//////////////////////////////////////////////////////////////////////////
	// Extract Material Information
	//////////////////////////////////////////////////////////////////////////

	// This tells what vertex buffer index belongs to what vertex shader input binding name
	generalMaterial->linkVertexBuffer("in_Position", pigMeshComponent->getMesh()->getVertexBufferIndex());
	generalMaterial->linkVertexBuffer("in_Color", pigMeshComponent->getMesh()->getColorBufferIndex());
	generalMaterial->linkVertexBuffer("in_Uvs", pigMeshComponent->getMesh()->getUvBufferIndex());

	// Do the same for the plane
	planeMaterial->linkVertexBuffer("in_Position", planeComponent->getMesh()->getVertexBufferIndex());
	planeMaterial->linkVertexBuffer("in_Color", planeComponent->getMesh()->getColorBufferIndex());
	planeMaterial->linkVertexBuffer("in_Uvs", planeComponent->getMesh()->getUvBufferIndex());

	// And sphere
	sphereMaterial->linkVertexBuffer("in_Position", sphereComponent->getMesh()->getVertexBufferIndex());
	sphereMaterial->linkVertexBuffer("in_Color", sphereComponent->getMesh()->getColorBufferIndex());
	sphereMaterial->linkVertexBuffer("in_Uvs", sphereComponent->getMesh()->getUvBufferIndex());

	// Orientation gizmo
	orientationMaterial->linkVertexBuffer("in_Position", orientationMeshComponent->getMesh()->getVertexBufferIndex());
	orientationMaterial->linkVertexBuffer("in_Color", orientationMeshComponent->getMesh()->getColorBufferIndex());

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

	// Camera used to split one view into multiple windows
	nap::Entity& split_camera_entity = core.addEntity("split_camera");
	splitCameraComponent = &split_camera_entity.addComponent<nap::CameraComponent>();
	nap::TransformComponent& split_camera_transform = split_camera_entity.addComponent<nap::TransformComponent>();
	split_camera_transform.translate.setValue({ 0.0f, 0.0f, 5.0f });
	splitCameraComponent->clippingPlanes.setValue(glm::vec2(0.01, 1000.0f));
	splitCameraComponent->fieldOfView.setValue(45.0f);
	splitCameraComponent->setAspectRatio((float)windowWidth * 2.0f, (float)windowHeight);
	splitCameraComponent->setGridDimensions(1, 2);

	return true;
}


void createSpheres(nap::Core& core, nap::Resource& shader)
{
	/*
	// Create sphere entity
	sphere = &(core.getRoot().addEntity("sphere"));

	float range = 20.0f;
	for (int i = 0; i < 100; i++)
	{
		nap::Entity& sphere_e = sphere->addEntity("sphere_child");

		nap::TransformComponent& sphere_tran_component = sphere_e.addComponent<nap::TransformComponent>();
		sphereComponent = &sphere_e.addComponent<nap::SphereComponent>("draw_sphere");

		float x = ((float)rand() / (float)RAND_MAX) * range;
		float y = ((float)rand() / (float)RAND_MAX) * range;
		float z = ((float)rand() / (float)RAND_MAX) * range;

		sphere_tran_component.translate.setValue(glm::vec3(x, y, z));

		nap::Material* sphere_material = sphereComponent->getMaterial();
		assert(sphere_material != nullptr);
		sphere_material->shaderResourceLink.setResource(shader);

		sphere_material->linkVertexBuffer("in_Position", sphereComponent->getMesh()->getVertexBufferIndex());
		sphere_material->linkVertexBuffer("in_Color", sphereComponent->getMesh()->getColorBufferIndex());
		sphere_material->linkVertexBuffer("in_Uvs", sphereComponent->getMesh()->getUvBufferIndex());
	}

	/*
	// Create sphere entity
	sphere = &(core.getRoot().addEntity("sphere"));
	nap::TransformComponent& sphere_tran_component = sphere->addComponent<nap::TransformComponent>();
	sphereComponent = &sphere->addComponent<nap::SphereComponent>("draw_sphere");

	nap::Material* sphere_material = sphereComponent->getMaterial();
	assert(sphere_material != nullptr);
	sphere_material->shaderResourceLink.setResource(*shader_resource);
	*/
}

struct PointeeBase
{
public:
	RTTI_ENABLE()
	// If you uncomment this line the code will no longer compile. If you leave it commented, the code will compile but crash
	//Pointee& operator=(const Pointee&) = delete;
};

struct PointeeDerived : public PointeeBase
{
public:
	RTTI_ENABLE(PointeeBase)
};

struct ClassWithPointer
{
public:
	PointeeDerived* mPointee = nullptr;
};

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<PointeeBase>("PointeeBase")
					.constructor<>()(policy::ctor::as_raw_ptr);

	registration::class_<PointeeDerived>("PointeeDerived")
		.constructor<>()(policy::ctor::as_raw_ptr);

	registration::class_<ClassWithPointer>("ClassWithPointer")
					.constructor<>()(policy::ctor::as_raw_ptr)
					.property("mPointee", &ClassWithPointer::mPointee);
}


// Main loop
int main(int argc, char *argv[])
{
	rttr::type obj_type = rttr::type::get<ClassWithPointer>();
	ClassWithPointer* obj = obj_type.create().get_value<ClassWithPointer*>();

	// This line will either crash, or if it doesn't crash, the mPointee member will still be null
	PointeeBase* new_pointee = RTTI_OF(PointeeDerived).create<PointeeBase>();
	rttr::variant variant = new_pointee;
	obj_type.get_property("mPointee").set_value(*obj, variant);
	obj_type.get_property("mPointee").set_value(*obj, nullptr);

	rttr::variant v = obj_type.get_property("mPointee").get_value(*obj);
	PointeeBase* test = v.get_value<PointeeBase*>(); 
// 	RTTI::TypeInfo renderService = rttr::type::get<nap::RenderService>();
// 	RTTI::TypeInfo service = rttr::type::get<nap::Service>();
// 	RTTI::TypeInfo service2 = RTTI_OF(nap::Service);
// 
// 	bool is_kind_of = renderService.isKindOf(service);
// 
// 	rttr::type test_class_type = rttr::type::get<test_class>();
// 	rttr::type test_class_type2 = rttr::type::get<test_class2>();
// 
// 	test_class* obj1 = test_class_type.createInstance<test_class>();
// 	test_class2* obj2 = test_class_type2.createInstance<test_class2>();
// 
// 	rttr::property prop = test_class_type.get_property("value");
// 	prop.set_value(*obj1, 42);

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
				case SDLK_0:
				{
					cameraComponent->lookAt.clear();
					break;
				}
				case SDLK_1:
				{
					cameraComponent->lookAt.setTarget(*pigMeshComponent);
					break;
				}
				case SDLK_2:
				{
					cameraComponent->lookAt.setTarget(*planeComponent);
					break;
				}
				case SDLK_3:
				{
					cameraComponent->lookAt.setTarget(*sphereComponent);
					break;
				}
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
 