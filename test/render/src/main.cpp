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
#include <renderablemeshcomponent.h>
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

nap::ObjectPtr<nap::TextureRenderTargetResource2D> textureRenderTarget;
nap::CameraComponent* cameraComponent = nullptr;
nap::CameraComponent* splitCameraComponent = nullptr;
nap::RenderableMeshComponent* pigMeshComponent = nullptr;
nap::PlaneComponent* rotatingPlaneComponent = nullptr;
nap::PlaneComponent* planeComponent = nullptr;
nap::SphereComponent* sphereComponent = nullptr;
nap::ObjectPtr<nap::MeshResource> pigMesh = nullptr;
nap::ObjectPtr<nap::MeshResource> orientationMesh = nullptr;
nap::RenderableMeshComponent* orientationMeshComponent = nullptr;
nap::ObjectPtr<nap::MaterialInstance> pigMaterialInstance = nullptr;
nap::ObjectPtr<nap::MaterialInstance> planeMaterialInstance = nullptr;
nap::ObjectPtr<nap::MaterialInstance> rotatingPlaneMaterialInstance = nullptr;
nap::ObjectPtr<nap::MaterialInstance> worldMaterialInstance = nullptr;
nap::ObjectPtr<nap::MaterialInstance> orientationMaterialInstance = nullptr;

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
nap::Entity* rotating_plane  = nullptr;
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
	nap::TransformComponent* xform_p = rotatingPlaneComponent->getParent()->getComponent<nap::TransformComponent>();
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
	nap::MaterialInstance* material_instance = pigMeshComponent->getMaterialInstance();

	float v = (sin(elapsed_time) + 1.0f) / 2.0f;

	// Set uniforms
	glm::vec4 color(v, 1.0f-v, 1.0f, 1.0f);
	material_instance->getOrCreateUniform<nap::UniformVec4>("mColor").setValue(color);

	// Set plane uniforms
	nap::MaterialInstance* plane_material = planeComponent->getMaterialInstance();
	plane_material->getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*testTexture);
	plane_material->getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(*testTexture);
	plane_material->getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
	plane_material->getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

	nap::MaterialInstance* rotating_plane_material = rotatingPlaneComponent->getMaterialInstance();
	rotating_plane_material->getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*testTexture);
	rotating_plane_material->getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(*testTexture);
	rotating_plane_material->getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
	rotating_plane_material->getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

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
	renderService->destroyGLContextResources(renderWindows);

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
		components_to_render.push_back(rotatingPlaneComponent);

		nap::MaterialInstance* plane_material = planeComponent->getMaterialInstance();
		plane_material->getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material->getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material->getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		plane_material->getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		nap::MaterialInstance* rotating_plane_material = rotatingPlaneComponent->getMaterialInstance();
		rotating_plane_material->getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material->getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material->getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		rotating_plane_material->getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

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


bool initResources(nap::utility::ErrorState& errorState)
{
	pigTexture = resourceManagerService->createResource<nap::ImageResource>();
	pigTexture->mImagePath = pigTextureName;
	if (!pigTexture->init(errorState))
		return false;
	
	testTexture = resourceManagerService->createResource<nap::ImageResource>();
	testTexture->mImagePath = testTextureName;
	if (!testTexture->init(errorState))
		return false;

	worldTexture = resourceManagerService->createResource<nap::ImageResource>();
	worldTexture->mImagePath = worldTextureName;
	if (!worldTexture->init(errorState))
		return false;

	nap::MemoryTextureResource2D* color_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	color_texture->mSettings.width = 640;
	color_texture->mSettings.height = 480;
	color_texture->mSettings.internalFormat = GL_RGBA;
	color_texture->mSettings.format = GL_RGBA;
	color_texture->mSettings.type = GL_UNSIGNED_BYTE;
	if (!color_texture->init(errorState))
		return false;

	nap::MemoryTextureResource2D* depth_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	depth_texture->mSettings.width = 640;
	depth_texture->mSettings.height = 480;
	depth_texture->mSettings.internalFormat = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.format = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.type = GL_FLOAT;
	if (!depth_texture->init(errorState))
		return false;
	
	// Create frame buffer
	textureRenderTarget = resourceManagerService->createResource<nap::TextureRenderTargetResource2D>();
	textureRenderTarget->setColorTexture(*color_texture);
	textureRenderTarget->setDepthTexture(*depth_texture);
	textureRenderTarget->mClearColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	if (!textureRenderTarget->init(errorState))
		return false;

	// Load general shader
	nap::ShaderResource* generalShaderResource = resourceManagerService->createResource<nap::ShaderResource>();
	generalShaderResource->mVertPath = vertShaderName;
	generalShaderResource->mFragPath = fragShaderName;
	if (!generalShaderResource->init(errorState))
		return false;

	// Load orientation shader
	nap::ShaderResource* orientationShaderResource = resourceManagerService->createResource<nap::ShaderResource>();
	orientationShaderResource->mVertPath = orientationVertShaderName;
	orientationShaderResource->mFragPath = orientationFragShaderName;
	if (!orientationShaderResource->init(errorState))
		return false;

	// Load orientation resource
	nap::MeshResource* orientationMesh = resourceManagerService->createResource<nap::MeshResource>();
	orientationMesh->mPath = "data/orientation.mesh";
	if (!orientationMesh->init(errorState))
		return false;

	// Load mesh resource
	nap::MeshResource* pigMesh = resourceManagerService->createResource<nap::MeshResource>();
	pigMesh->mPath = "data/pig_head_alpha_rotated.mesh";
	if (!pigMesh->init(errorState))
		return false;
	/*
	nap::Material* pigMaterial = resourceManagerService->createResource<nap::Material>();
	pigMaterial->mShader = generalShaderResource;
	pigMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!pigMaterial->init(errorState))
		return false;

	generalMaterial = resourceManagerService->createResource<nap::Material>();
	generalMaterial->mShader = generalShaderResource;
	generalMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!generalMaterial->init(errorState))
		return false;

	worldMaterial = resourceManagerService->createResource<nap::Material>();
	worldMaterial->mShader = generalShaderResource;
	worldMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!worldMaterial->init(errorState))
		return false;

	nap::Material* orientationMaterial = resourceManagerService->createResource<nap::Material>();
	orientationMaterial->mShader = orientationShaderResource;
	orientationMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!orientationMaterial->init(errorState))
		return false;

	pigRenderableMesh = resourceManagerService->createResource<nap::RenderableMeshResource>();
	pigRenderableMesh->mMaterialResource = pigMaterial;
	pigRenderableMesh->mMeshResource = pigMesh;
	if (!pigRenderableMesh->init(errorState))
		return false;

	orientationRenderableMesh = resourceManagerService->createResource<nap::RenderableMeshResource>();
	orientationRenderableMesh->mMaterialResource = orientationMaterial;
	orientationRenderableMesh->mMeshResource = orientationMesh;
	if (!orientationRenderableMesh->init(errorState))
		return false;
		*/
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
	rtti::TypeInfo rpcServiceType = rtti::TypeInfo::getByName(rpcServiceTypename);
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

	// Get resource manager service
	resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();

	// Create render service
	renderService = core.getOrCreateService<nap::RenderService>();
	
	// TODO: Init should be without arguments and called by core when added to the system (COEN)
	// Problem is custom service arguments such as the one below (render type), maybe have a settings construct for
	// services?
	nap::utility::ErrorState error;
	if (!renderService->init(RTTI_OF(nap::OpenGLRenderer), error))
	{
		nap::Logger::fatal(error.toString());
		return false;
	}

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

	nap::utility::ErrorState errorState;
#if 1
	if (!resourceManagerService->loadFile("data/objects.json", errorState))
	{
		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
		return false;
	}

	pigTexture = resourceManagerService->findResource<nap::ImageResource>("PigTexture");
 	testTexture = resourceManagerService->findResource<nap::ImageResource>("TestTexture");
 	worldTexture = resourceManagerService->findResource<nap::ImageResource>("WorldTexture");
 	textureRenderTarget = resourceManagerService->findResource<nap::TextureRenderTargetResource2D>("PlaneRenderTarget");
	pigMaterialInstance = resourceManagerService->findResource<nap::MaterialInstance>("PigMaterialInstance");
	planeMaterialInstance = resourceManagerService->findResource<nap::MaterialInstance>("PlaneMaterialInstance");
	orientationMaterialInstance = resourceManagerService->findResource<nap::MaterialInstance>("OrientationMaterialInstance");
	rotatingPlaneMaterialInstance = resourceManagerService->findResource<nap::MaterialInstance>("RotatingPlaneMaterialInstance");
	worldMaterialInstance = resourceManagerService->findResource<nap::MaterialInstance>("WorldMaterialInstance");
	pigMesh = resourceManagerService->findResource<nap::MeshResource>("PigMesh");
	orientationMesh = resourceManagerService->findResource<nap::MeshResource>("OrientationMesh");
#else	
	if (!initResources(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
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
	pigMeshComponent = &model->addComponent<nap::RenderableMeshComponent>("pig_head_mesh");
	pigMeshComponent->mMaterialInstance = pigMaterialInstance;
	pigMeshComponent->mMeshResource = pigMesh;
	if (!pigMeshComponent->init(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
		return false;
	}

	// Create orientation entity
	orientation = &(core.getRoot().addEntity("orientation"));
	nap::TransformComponent& or_tran_component = orientation->addComponent<nap::TransformComponent>();
	or_tran_component.uniformScale.setValue(0.33f);
	orientationMeshComponent = &orientation->addComponent<nap::RenderableMeshComponent>("orientation gizmo");
	orientationMeshComponent->mMeshResource = orientationMesh;
	orientationMeshComponent->mMaterialInstance = orientationMaterialInstance;
	if (!orientationMeshComponent->init(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
		return false;
	}

	// Create plane entity
	rotating_plane = &(core.getRoot().addEntity("rotating_plane"));
	rotating_plane->addComponent<nap::TransformComponent>();
	rotatingPlaneComponent = new nap::PlaneComponent(*rotatingPlaneMaterialInstance);
	rotating_plane->addComponent(std::move(std::unique_ptr<nap::Component>(rotatingPlaneComponent)));
	if (!rotatingPlaneComponent->init(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
		return false;
	}

	// Create plane entity
	plane = &(core.getRoot().addEntity("plane"));
	plane->addComponent<nap::TransformComponent>();
	planeComponent = new nap::PlaneComponent(*planeMaterialInstance);
	plane->addComponent(std::move(std::unique_ptr<nap::Component>(planeComponent)));
	if (!planeComponent->init(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
		return false;
	}

	// Create sphere entity
	sphere = &(core.getRoot().addEntity("sphere"));
	nap::TransformComponent& sphere_tran_component = sphere->addComponent<nap::TransformComponent>();
	sphereComponent = new nap::SphereComponent(*worldMaterialInstance);
	sphere->addComponent(std::move(std::unique_ptr<nap::Component>(sphereComponent)));
	if (!sphereComponent->init(errorState))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", errorState.toString().c_str());
		return false;
	}

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
					cameraComponent->lookAt.setTarget(*rotatingPlaneComponent);
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
       
