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
#include <rendertargetresource.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/coreattributes.h>

// STD includes
#include <ctime>


#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>



//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Window Name
std::string		programName			= "Model Loading Test";
std::string		vertShaderName		= "shaders/shader.vert";
std::string		fragShaderName		= "shaders/shader.frag";
std::string		vertShaderNameTwo	= "shaders/shader_two.vert";
std::string		fragShaderNameTwo	= "shaders/shader_two.frag";
std::string		orientationShaderName = "shaders/orientation.frag";

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
nap::Service* rpcService = nullptr;
std::vector<nap::RenderWindowComponent*> renderWindows;
nap::TextureRenderTargetResource2D* textureRenderTarget;
nap::CameraComponent* cameraComponent = nullptr;
nap::CameraComponent* splitCameraComponent = nullptr;
nap::ModelMeshComponent* modelComponent = nullptr;
nap::PlaneComponent* planeComponent = nullptr;
nap::SphereComponent* sphereComponent = nullptr;
nap::ShaderResource* shaderResource = nullptr;
nap::ShaderResource* orientationShaderResource = nullptr;
nap::ModelMeshComponent* orientationComponent = nullptr;

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
	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();
	static float prev_elapsed_time = elapsed_time;
	float delta_time = prev_elapsed_time - elapsed_time;
	if (delta_time < 0.01f)
	{
		delta_time = 0.01f;
	}

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
		components_to_render.push_back(modelComponent);

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

using ObjectMap = std::unordered_map<std::string, nap::Object*>;

bool readObjects(rapidjson::Document& inDocument, nap::ResourceManagerService* resourceManagerService, ObjectMap& objects, nap::InitResult& initResult)
{
	std::unordered_map<nap::ObjectLinkAttribute*, std::string> links_to_resolve;

	for (auto& object_pos = inDocument.MemberBegin(); object_pos < inDocument.MemberEnd(); ++object_pos)
	{
		const char* typeName = object_pos->name.GetString();
		RTTI::TypeInfo type_info = RTTI::TypeInfo::getByName(typeName);
		if (!initResult.check(type_info.isValid(), "Unknown object type %s encountered.", typeName))
			return false;

		if (!initResult.check(type_info.canCreateInstance(), "Unable to instantiate object of type %s.", typeName))
			return false;

		if (!initResult.check(type_info.isKindOf(RTTI_OF(nap::Resource)), "Unable to instantiate object %s. Class is not derived from Resource.", typeName))
			return false;

		nap::Resource* resource = resourceManagerService->createResource(type_info);

		for (auto& member_pos = object_pos->value.MemberBegin(); member_pos < object_pos->value.MemberEnd(); ++member_pos)
		{
			const char* attrName = member_pos->name.GetString();
			nap::Object* child = resource->getChild(attrName);
			nap::AttributeBase* attribute = rtti_cast<nap::AttributeBase>(child);

			if (attribute == nullptr)
				continue;

			if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::Attribute<std::string>)))
			{
				((nap::Attribute<std::string>*)attribute)->setValue(member_pos->value.GetString());
			}
			else if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<int>)))
			{
				((nap::NumericAttribute<int>*)attribute)->setValue(member_pos->value.GetInt());
			}
			else if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::ObjectLinkAttribute)))
			{
				links_to_resolve.insert({ (nap::ObjectLinkAttribute*)attribute, std::string(member_pos->value.GetString()) });
			}
		}
		
		std::string id = resource->mID.getValue();
		objects.insert(std::make_pair(id, resource));
	}

	for (auto kvp : links_to_resolve)
	{
		ObjectMap::iterator target = objects.find(kvp.second);

		if (!initResult.check(target != objects.end(), "Unable to resolve link to object %s from attribute %s", kvp.second.c_str(), kvp.first->getName().c_str()))
			return false;

		kvp.first->setTarget(*target->second);
	}

	for (auto kvp : objects)
	{
		nap::Resource* resource = rtti_cast<nap::Resource>(kvp.second);
		if (!resource->init(initResult))
			return false;
	}
		
	return true;
}

bool initFromFile(const std::string& filename, nap::ResourceManagerService* resourceManagerService, std::unordered_map<std::string, nap::Object*>& objects, nap::InitResult& initResult)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!initResult.check(in.good(), "Unable to open file %s", filename.c_str()))
		return false;

	// Create buffer of appropriate size
	in.seekg(0, std::ios::end);
	size_t len = in.tellg();
	std::string buffer;
	buffer.resize(len);

	// Read all data
	in.seekg(0, std::ios::beg);
	in.read(&buffer[0], len);
	in.close();

	// Parse document
	rapidjson::Document doc;
	rapidjson::ParseResult parse_result = doc.ParseInsitu((char*)buffer.c_str());

	if (!parse_result)
	{
		size_t start = buffer.rfind('\n', parse_result.Offset());
		size_t end = buffer.find('\n', parse_result.Offset());

		if (start == std::string::npos)
			start = 0;
		if (end == std::string::npos)
			end = buffer.size();
		
		std::string error_line = buffer.substr(start, end - start);

		initResult.mErrorString = nap::stringFormat("Error parsing %s: %s (line: %s)", filename.c_str(), rapidjson::GetParseError_En(parse_result.Code()), error_line.c_str());
		return false;
	} 

	// Read data
	if (!readObjects(doc, resourceManagerService, objects, initResult))
		return false;	

	return true;
}

bool initResources(nap::ResourceManagerService* resourceManagerService, nap::InitResult& initResult)
{
	pigTexture = resourceManagerService->createResource<nap::ImageResource>();
	pigTexture->mImagePath.setValue(pigTextureName);
	if (!pigTexture->init(initResult))
		return false;

	testTexture = resourceManagerService->createResource<nap::ImageResource>();
	testTexture->mImagePath.setValue(testTextureName);
	if (!testTexture->init(initResult))
		return false;

	worldTexture = resourceManagerService->createResource<nap::ImageResource>();
	worldTexture->mImagePath.setValue(worldTextureName);
	if (!worldTexture->init(initResult))
		return false;

	nap::MemoryTextureResource2D* color_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	color_texture->mWidth.setValue(640);
	color_texture->mHeight.setValue(480);
	color_texture->mInternalFormat.setValue(GL_RGBA);
	color_texture->mFormat.setValue(GL_RGBA);
	color_texture->mType.setValue(GL_UNSIGNED_BYTE);
	if (!color_texture->init(initResult))
		return false;

	nap::MemoryTextureResource2D* depth_texture = resourceManagerService->createResource<nap::MemoryTextureResource2D>();
	depth_texture->mWidth.setValue(640);
	depth_texture->mHeight.setValue(480);
	depth_texture->mInternalFormat.setValue(GL_DEPTH_COMPONENT);
	depth_texture->mFormat.setValue(GL_DEPTH_COMPONENT);
	depth_texture->mType.setValue(GL_FLOAT);
	if (!depth_texture->init(initResult))
		return false;
	
	// Create frame buffer
	textureRenderTarget = resourceManagerService->createResource<nap::TextureRenderTargetResource2D>();
	textureRenderTarget->setColorTexture(*color_texture);
	textureRenderTarget->setDepthTexture(*depth_texture);
	if (!textureRenderTarget->init(initResult))
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
		nap::RenderWindowComponent* renderWindow = RTTI_OF(nap::RenderWindowComponent).createInstance<nap::RenderWindowComponent>();

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

	// Create shader resource
	nap::ResourceManagerService* resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
	resourceManagerService->setAssetRoot(".");

	// Load orientation resource
	nap::ModelResource* orientation_model = resourceManagerService->getResource<nap::ModelResource>("data/orientation.fbx");
	assert(orientation_model != nullptr);
	orientation_model->load();

	// Load model resource
	nap::ModelResource* pig_model = resourceManagerService->getResource<nap::ModelResource>("data/pig_head_alpha_rotated.fbx");
	assert(orientation_model != nullptr);
	pig_model->load();

#if 1
	std::unordered_map<std::string, nap::Object*> objects;
	nap::InitResult initResult;
	if (!initFromFile("data/objects.json", resourceManagerService, objects, initResult))
	{
		nap::Logger::fatal("Unable to deserialize resources: %s", initResult.mErrorString.c_str());
		return false;
	}

	pigTexture = rtti_cast<nap::ImageResource>(objects.find("PigTexture")->second);
	testTexture = rtti_cast<nap::ImageResource>(objects.find("TestTexture")->second);
	worldTexture = rtti_cast<nap::ImageResource>(objects.find("WorldTexture")->second);
	textureRenderTarget = rtti_cast<nap::TextureRenderTargetResource2D>(objects.find("PlaneRenderTarget")->second);
#else	
	nap::InitResult initResult;
	if (!initResources(resourceManagerService, initResult))
	{
		nap::Logger::fatal("Unable to initialize resources: %s", initResult.mErrorString.c_str());
		return false;
	}
#endif

	textureRenderTarget->getTarget().setClearColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	// Load general shader
	shaderResource = resourceManagerService->getResource<nap::ShaderResource>(fragShaderName);
	shaderResource->load();

	// Load orientation shader
	orientationShaderResource = resourceManagerService->getResource<nap::ShaderResource>(orientationShaderName);
	orientationShaderResource->load();

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
	modelComponent = &model->addComponent<nap::ModelMeshComponent>("pig_head_mesh");

	// Create orientation entity
	orientation = &(core.getRoot().addEntity("orientation"));
	nap::TransformComponent& or_tran_component = orientation->addComponent<nap::TransformComponent>();
	or_tran_component.uniformScale.setValue(0.33f);
	orientationComponent = &orientation->addComponent<nap::ModelMeshComponent>("orientation gizmo");

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

	// Plane material
	nap::Material* plane_material = planeComponent->getMaterial();
	assert(plane_material != nullptr);
	plane_material->shaderResourceLink.setResource(*shaderResource);

	// Sphere material
	nap::Material* sphere_material = sphereComponent->getMaterial();
	assert(sphere_material != nullptr);
	sphere_material->shaderResourceLink.setResource(*shaderResource);

	// Orientation material
	nap::Material* orientation_material = orientationComponent->getMaterial();
	orientation_material->shaderResourceLink.setResource(*orientationShaderResource);

	// Link model resource
	modelComponent->modelResource.setResource(*pig_model);

	// Link orientation resource
	orientationComponent->modelResource.setResource(*orientation_model);

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

	// Extract orientation mesh
	const opengl::Mesh* orientation_mesh = orientation_model->getMesh(0);
	if (orientation_mesh == nullptr)
	{
		nap::Logger::warn("unable to extract orientation mesh at index 0");
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

	// Orientation gizmo
	orientation_material->linkVertexBuffer("in_Position", orientationComponent->getMesh()->getVertexBufferIndex());
	orientation_material->linkVertexBuffer("in_Color", orientationComponent->getMesh()->getColorBufferIndex());

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
					cameraComponent->lookAt.setTarget(*modelComponent);
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
