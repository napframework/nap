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
#include <planemeshresource.h>
#include <spheremeshresource.h>
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
static nap::ObjectPtr<nap::ImageResource> testTexture = nullptr;
static const std::string pigTextureName = "data/pig_head.jpg";
static nap::ObjectPtr<nap::ImageResource> pigTexture = nullptr;
static const std::string worldTextureName = "data/world_texture.jpg";
static nap::ObjectPtr<nap::ImageResource> worldTexture = nullptr;
static float movementScale = 3.0f;
static float rotateScale = 1.0f;

// Nap Objects
nap::RenderService* renderService = nullptr;
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::Service* rpcService = nullptr;
std::vector<nap::RenderWindowComponent*> renderWindows;

nap::ObjectPtr<nap::TextureRenderTargetResource2D>	textureRenderTarget;
nap::ObjectPtr<nap::EntityInstance>					pigEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					rotatingPlaneEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					planeEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					worldEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					orientationEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					cameraEntity = nullptr;
nap::ObjectPtr<nap::EntityInstance>					splitCameraEntity = nullptr;

// movement
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
	nap::MaterialInstance* material_instance = pigEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();

	float v = (sin(elapsed_time) + 1.0f) / 2.0f;

	// Set uniforms
	glm::vec4 color(v, 1.0f-v, 1.0f, 1.0f);
	material_instance->getOrCreateUniform<nap::UniformVec4>("mColor").setValue(color);

	// Set plane uniforms
	nap::MaterialInstance* plane_material = planeEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
	plane_material->getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*testTexture);
	plane_material->getOrCreateUniform<nap::UniformTexture2D>("testTexture").setTexture(*testTexture);
	plane_material->getOrCreateUniform<nap::UniformInt>("mTextureIndex").setValue(0);
	plane_material->getOrCreateUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

	nap::MaterialInstance* rotating_plane_material = rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
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
	//cam_xform->setRotate(nap::quatToVector(rot_quat));
}
nap::Slot<const nap::SignalAttribute&> updateSlot = { [](const nap::SignalAttribute& attr){ onUpdate(attr); } };


void updateCamera(float deltaTime)
{
	float movement = movementScale * deltaTime;
	float rotate = rotateScale * deltaTime;
	float rotate_rad = rotate;

	nap::TransformComponent* cam_xform = &cameraEntity->getComponent<nap::TransformComponent>();
	//glm::vec3 lookat_pos = cam_xform->getGlobalTransform()[0];
	//glm::vec3 dir = glm::cross(glm::normalize(lookat_pos), glm::vec3(cam_xform->getGlobalTransform()[1]));
	//glm::vec3 dir_f = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0,1.0,0.0));
	//glm::vec3 dir_s = glm::cross(glm::normalize(lookat_pos), glm::vec3(0.0, 0.0, 1.0));
	//dir_f *= movement;
	//dir_s *= movement;

	glm::vec3 side(1.0, 0.0, 0.0);
	glm::vec3 forward(0.0, 0.0, 1.0);

	glm::vec3 dir_forward = glm::rotate(cam_xform->getRotate(),forward);
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

	// Render window 0
	{
		nap::RenderWindowComponent* render_window = renderWindows[0];

		render_window->makeActive();
		
		// Render entire scene to texture
		renderService->clearRenderTarget(textureRenderTarget->getTarget(), opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		renderService->renderObjects(textureRenderTarget->getTarget(), cameraEntity->getComponent<nap::CameraComponent>());

		// Render output texture to plane
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(&planeEntity->getComponent<nap::RenderableMeshComponent>());
		components_to_render.push_back(&rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>());

		nap::MaterialInstance* plane_material = planeEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
		plane_material->getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material->getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		plane_material->getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		plane_material->getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		nap::MaterialInstance* rotating_plane_material = rotatingPlaneEntity->getComponent<nap::RenderableMeshComponent>().getMaterialInstance();
		rotating_plane_material->getUniform<nap::UniformTexture2D>("testTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material->getUniform<nap::UniformTexture2D>("pigTexture").setTexture(textureRenderTarget->GetColorTexture());
		rotating_plane_material->getUniform<nap::UniformInt>("mTextureIndex").setValue(0);
		rotating_plane_material->getUniform<nap::UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR|opengl::EClearFlags::DEPTH|opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, components_to_render, cameraEntity->getComponent<nap::CameraComponent>());

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::CameraComponent>().setGridLocation(0, 0);
		components_to_render.clear();
		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponent>());
		renderService->renderObjects(backbuffer, components_to_render, splitCameraEntity->getComponent<nap::CameraComponent>());

		render_window->swap();
	}
	 
	// render window 1
	{
		nap::RenderWindowComponent* render_window = renderWindows[1];

		render_window->makeActive();

		// Render specific object directly to backbuffer
		std::vector<nap::RenderableComponent*> components_to_render;
		components_to_render.push_back(&pigEntity->getComponent<nap::RenderableMeshComponent>());

		opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(render_window->getWindow()->getBackbuffer());
		renderService->clearRenderTarget(backbuffer, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH | opengl::EClearFlags::STENCIL);
		renderService->renderObjects(backbuffer, components_to_render, cameraEntity->getComponent<nap::CameraComponent>());

		// Render sphere using split camera with custom projection matrix
		splitCameraEntity->getComponent<nap::CameraComponent>().setGridLocation(0, 1);
 		components_to_render.clear();
 		components_to_render.push_back(&worldEntity->getComponent<nap::RenderableMeshComponent>());
 		renderService->renderObjects(backbuffer, components_to_render, splitCameraEntity->getComponent<nap::CameraComponent>());

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

#if 0
bool initResources(nap::utility::ErrorState& errorState)
{
	pigTexture = resourceManagerService->createObject<nap::ImageResource>();
	pigTexture->mImagePath = pigTextureName;
	if (!pigTexture->init(errorState))
		return false;
	
	testTexture = resourceManagerService->createObject<nap::ImageResource>();
	testTexture->mImagePath = testTextureName;
	if (!testTexture->init(errorState))
		return false;

	worldTexture = resourceManagerService->createObject<nap::ImageResource>();
	worldTexture->mImagePath = worldTextureName;
	if (!worldTexture->init(errorState))
		return false;

	nap::ObjectPtr<nap::MemoryTextureResource2D> color_texture = resourceManagerService->createObject<nap::MemoryTextureResource2D>();
	color_texture->mSettings.width = 640;
	color_texture->mSettings.height = 480;
	color_texture->mSettings.internalFormat = GL_RGBA;
	color_texture->mSettings.format = GL_RGBA;
	color_texture->mSettings.type = GL_UNSIGNED_BYTE;
	if (!color_texture->init(errorState))
		return false;

	nap::ObjectPtr<nap::MemoryTextureResource2D> depth_texture = resourceManagerService->createObject<nap::MemoryTextureResource2D>();
	depth_texture->mSettings.width = 640;
	depth_texture->mSettings.height = 480;
	depth_texture->mSettings.internalFormat = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.format = GL_DEPTH_COMPONENT;
	depth_texture->mSettings.type = GL_FLOAT;
	if (!depth_texture->init(errorState))
		return false;
	
	// Create frame buffer
	textureRenderTarget = resourceManagerService->createObject<nap::TextureRenderTargetResource2D>();
	textureRenderTarget->setColorTexture(*color_texture);
	textureRenderTarget->setDepthTexture(*depth_texture);
	textureRenderTarget->mClearColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	if (!textureRenderTarget->init(errorState))
		return false;

	// Load general shader
	nap::ObjectPtr<nap::ShaderResource> generalShaderResource = resourceManagerService->createObject<nap::ShaderResource>();
	generalShaderResource->mVertPath = vertShaderName;
	generalShaderResource->mFragPath = fragShaderName;
	if (!generalShaderResource->init(errorState))
		return false;

	// Load orientation shader
	nap::ObjectPtr<nap::ShaderResource> orientationShaderResource = resourceManagerService->createObject<nap::ShaderResource>();
	orientationShaderResource->mVertPath = orientationVertShaderName;
	orientationShaderResource->mFragPath = orientationFragShaderName;
	if (!orientationShaderResource->init(errorState))
		return false;

	// Load orientation resource
	orientationMesh = resourceManagerService->createObject<nap::MeshFromFileResource>();
	orientationMesh->mPath = "data/orientation.mesh";
	if (!orientationMesh->init(errorState))
		return false;

	// Load mesh resource
	pigMesh = resourceManagerService->createObject<nap::MeshFromFileResource>();
	pigMesh->mPath = "data/pig_head_alpha_rotated.mesh";
	if (!pigMesh->init(errorState))
		return false;

	planeMesh = resourceManagerService->createObject<nap::PlaneMeshResource>();
	if (!planeMesh->init(errorState))
		return false;

	sphereMesh = resourceManagerService->createObject<nap::SphereMeshResource>();
	if (!sphereMesh->init(errorState))
		return false;

	nap::ObjectPtr<nap::Material> orientationMaterial = resourceManagerService->createObject<nap::Material>();
	orientationMaterial->mShader = orientationShaderResource;
	orientationMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!orientationMaterial->init(errorState))
		return false;

	nap::ObjectPtr<nap::Material> generalMaterial = resourceManagerService->createObject<nap::Material>();
	generalMaterial->mShader = generalShaderResource;
	generalMaterial->mVertexAttributeBindings = nap::Material::getDefaultVertexAttributeBindings();
	if (!generalMaterial->init(errorState))
		return false;
	generalMaterial->getUniform<nap::UniformVec4>("mColor").setValue(glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

	pigMaterialInstance = resourceManagerService->createObject<nap::MaterialInstance>();
	pigMaterialInstance->mMaterial = generalMaterial;
	if (!pigMaterialInstance->init(errorState))
		return false;
	pigMaterialInstance->getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*pigTexture);

	planeMaterialInstance = resourceManagerService->createObject<nap::MaterialInstance>();
	planeMaterialInstance->mMaterial = generalMaterial;
	if (!planeMaterialInstance->init(errorState))
		return false;

	rotatingPlaneMaterialInstance = resourceManagerService->createObject<nap::MaterialInstance>();
	rotatingPlaneMaterialInstance->mMaterial = generalMaterial;
	if (!rotatingPlaneMaterialInstance->init(errorState))
		return false;

	orientationMaterialInstance = resourceManagerService->createObject<nap::MaterialInstance>();
	orientationMaterialInstance->mMaterial = orientationMaterial;
	if (!orientationMaterialInstance->init(errorState))
		return false;

	worldMaterialInstance = resourceManagerService->createObject<nap::MaterialInstance>();
	worldMaterialInstance->mMaterial = generalMaterial;
	if (!worldMaterialInstance->init(errorState))
		return false;
	worldMaterialInstance->getOrCreateUniform<nap::UniformTexture2D>("pigTexture").setTexture(*worldTexture);

	return true;
}
#endif

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

	pigTexture				= resourceManagerService->findObject<nap::ImageResource>("PigTexture");
 	testTexture				= resourceManagerService->findObject<nap::ImageResource>("TestTexture");
 	worldTexture			= resourceManagerService->findObject<nap::ImageResource>("WorldTexture");
 	textureRenderTarget		= resourceManagerService->findObject<nap::TextureRenderTargetResource2D>("PlaneRenderTarget");

	pigEntity				= resourceManagerService->findEntity("PigEntity");
	rotatingPlaneEntity		= resourceManagerService->findEntity("RotatingPlaneEntity");
	planeEntity				= resourceManagerService->findEntity("PlaneEntity");
	worldEntity				= resourceManagerService->findEntity("WorldEntity");
	orientationEntity		= resourceManagerService->findEntity("OrientationEntity");
	cameraEntity			= resourceManagerService->findEntity("CameraEntity");
	splitCameraEntity		= resourceManagerService->findEntity("SplitCameraEntity");
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

	cameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)windowWidth, (float)windowHeight);
	splitCameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)windowWidth * 2.0f, (float)windowHeight);

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
					//cameraComponent->lookAt.clear();
					break;
				}
				case SDLK_1:
				{
					//cameraComponent->lookAt.setTarget(*pigMeshComponent);
					break;
				}
				case SDLK_2:
				{
					//cameraComponent->lookAt.setTarget(*rotatingPlaneComponent);
					break;
				}
				case SDLK_3:
				{
					//cameraComponent->lookAt.setTarget(*sphereComponent);
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

					cameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)width, (float)height);
					splitCameraEntity->getComponent<nap::CameraComponent>().setAspectRatio((float)width * 2.0f, (float)height);
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
       
