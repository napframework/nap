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
#include <modelmeshcomponent.h>
#include <renderservice.h>
#include <renderwindowcomponent.h>
#include <openglrenderer.h>
#include <transformcomponent.h>
#include <cameracomponent.h>
#include <mathutils.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>

// STD includes
#include <ctime>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Window Name
std::string		programName		= "Model Loading Test";
std::string		vertShaderName	= "shaders/shader.vert";
std::string		fragShaderName	= "shaders/shader.frag";

static const std::string testTextureName = "data/test.jpg";
static std::unique_ptr<opengl::Image> testTexture;
static const std::string pigTextureName = "data/pig_head.jpg";
static std::unique_ptr<opengl::Image> pigTexture;

// Vertex buffer that holds all the fbo's
opengl::VertexArrayObject	cubeObject;

// Vertex buffer that holds a triangle
opengl::VertexArrayObject	triangleObject;

nap::Service*				rpcService = nullptr;

// Shader uniform bind locations
int	projectionMatrixLocation(-1);
int	viewMatrixLocation(-1);
int	modelMatrixLocation(-1);
int	noiseLocation(-1);
int textureLocation(-1);

// Render service and window
nap::RenderService* renderService = nullptr;
nap::RenderWindowComponent* renderWindow = nullptr;
nap::CameraComponent* cameraComponent = nullptr;
nap::ModelMeshComponent* modelComponent = nullptr;

// vertex Shader indices
nap::Entity* model = nullptr;
int vertex_index(0), color_index(0), normal_index(0), uv_index(0);

// Current texture to draw
unsigned int				currentIndex = 0;

// Window width / height on startup
unsigned int windowWidth(512);
unsigned int windowHeight(512);

// GLM
glm::mat4 modelMatrix;			// Store the model matrix

// Some utilities
void runGame(nap::Core& core);	
bool loadImages();
void updateViewport(int width, int height);

/**
* Loads a bitmap from disk
*/
bool loadImages()
{
	// Load blend image
	testTexture = std::make_unique<opengl::Image>(testTextureName);
	testTexture->setCompressed(true);
	if (!testTexture->load())
	{
		opengl::printMessage(opengl::MessageType::ERROR, "unable to load blend image: %s", testTextureName.c_str());
		return false;
	}

	pigTexture = std::make_unique<opengl::Image>(pigTextureName);
	pigTexture->setCompressed(true);
	if (!pigTexture->load())
	{
		opengl::printMessage(opengl::MessageType::ERROR, "unable to load pig texture: %s", pigTextureName.c_str());
		return false;
	}

	return true;
}

/**
* Updates gl viewport and projection matrix used for rendering
*/
void updateViewport(int width, int height)
{
	glViewport(0, 0, width, height);
	cameraComponent->setAspectRatio((float)width, (float)height);
}


// Called when the window is updating
void onUpdate(const nap::SignalAttribute& signal)
{
	//auto signal_attr = static_cast<nap::SignalAttribute*>(rpcService->getAttribute("update"));
	//signal_attr->trigger();

	// Update model transform
	float elapsed_time = renderService->getCore().getElapsedTime();

	// Get rotation angle
	float rot_speed = 0.5f;
	float rot_angle = elapsed_time * 360.0f * rot_speed;
	float rot_angle_radians = glm::radians(rot_angle);

	// Calculate rotation quaternion
	glm::quat rot_quat = glm::rotate(glm::quat(), (float)rot_angle_radians, glm::vec3(0.0, 1.0, 0.0));

	// Set rotation on component
	nap::TransformComponent* xform_v = modelComponent->getParent()->getComponent<nap::TransformComponent>();
	assert(xform_v);
	xform_v->rotate.setValue(nap::quatToVector(rot_quat));

	// Set transform
	float xform_distance = 2.0f;
	float xform_speed = 1.0f;
	float xform_offset = sin(elapsed_time * xform_speed) * xform_distance;
	xform_v->translate.setValue({ xform_offset, 0.0f, 0.0f });

	// Set scale
	float scale_speed = 4.0f;
	float nscale = (sin(elapsed_time  * scale_speed) + 1) / 2.0f;
	nscale = nap::fit<float>(nscale, 0.0f, 1.0f, 0.25f, 1.0f);
	xform_v->uniformScale.setValue(nscale);
}
NSLOT(updateSlot, const nap::SignalAttribute&, onUpdate)


// Called when the window is going to render
void onRender(const nap::SignalAttribute& signal)
{
	// Clear window
	opengl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
	opengl::clearDepth();
	opengl::clearStencil();

	// Enable some gl specific stuff
	opengl::enableDepthTest(true);
	opengl::enableBlending(true);
	opengl::enableMultiSampling(true);

	// Get mesh component
	nap::Material* material = modelComponent->getMaterial();
	assert(material != nullptr);

	// Bind Shader
	material->bind();

	// Get view matrix from camera
	nap::TransformComponent* cam_xform = cameraComponent->getParent()->getComponent<nap::TransformComponent>();
	assert(cam_xform != nullptr);

	// Get model matrix from model
	nap::TransformComponent* model_xform = modelComponent->getParent()->getComponent<nap::TransformComponent>();

	// Send values
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &cameraComponent->getProjectionMatrix()[0][0]);	// Send our projection matrix to the shader
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &cam_xform->getGlobalTransform()[0][0]);				// Send our view matrix to the shader
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &model_xform->getGlobalTransform()[0][0]);			// Send our model matrix to the shader

	// Set texture 1 for shader
	glActiveTexture(GL_TEXTURE0);

	// Bind correct texture and send to shader
	opengl::Image* img = currentIndex == 0 ? pigTexture.get() : testTexture.get();
	img->bind();
	glUniform1i(textureLocation, 0);

	// Unbind shader
	material->unbind();

	switch (currentIndex)
	{
	case 0:
		renderService->renderObjects();
		break;
	case 1:
		cubeObject.bind();
		material->bind();
		cubeObject.draw();
		material->unbind();
		cubeObject.unbind();
		break;
	case 2:
		triangleObject.bind();
		material->bind();
		triangleObject.draw();
		material->unbind();
		triangleObject.unbind();
		break;
	}
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
	renderWindow->sync.setValue(true);

	// Connect draw and update signals
	renderWindow->draw.signal.connect(renderSlot);
	renderWindow->update.signal.connect(updateSlot);

	//////////////////////////////////////////////////////////////////////////

	// Load bitmap
	if (!loadImages())
	{
		nap::Logger::fatal("unable to load images");
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Create Model
	//////////////////////////////////////////////////////////////////////////

	// Create shader resource
	nap::ResourceManagerService* service = core.getOrCreateService<nap::ResourceManagerService>();
	service->setAssetRoot(".");
	nap::Resource* shader_resource = service->getResource(fragShaderName);

	// Load model resource
	nap::Resource* model_resource = service->getResource("data/pig_head_alpha_rotated.fbx");
	if (model_resource == nullptr)
	{
		nap::Logger::warn("unable to load pig head model resource");
		return false;
	}
	nap::ModelResource* pig_model = static_cast<nap::ModelResource*>(model_resource);

	// Create model entity
	model = &(core.getRoot().addEntity("model"));
	nap::TransformComponent& tran_component = model->addComponent<nap::TransformComponent>();
	modelComponent = &model->addComponent<nap::ModelMeshComponent>("pig_head_mesh");

	//////////////////////////////////////////////////////////////////////////

	// Set shader resource on material
	nap::Material* material = modelComponent->getMaterial();
	assert(material != nullptr);
	material->shaderResource.setResource(*shader_resource);

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

	// Get buffer indices for mesh (TODO: RESOLVE DYNAMICALLY)
	vertex_index = mesh->getVertexBufferIndex();
	color_index = mesh->getColorBufferIndex(0);
	normal_index = mesh->getNormalBufferIndex();
	uv_index = mesh->getUvBufferIndex(0);

	// Bind indices explicit to shader (TODO: RESOLVE DYNAMICALLY)
	// This tells what vertex buffer index belongs to what vertex shader input binding name
	opengl::Shader& shader = material->getResource()->getShader();
	shader.bindVertexAttribute(vertex_index, "in_Position");
	shader.bindVertexAttribute(color_index, "in_Color");
	shader.bindVertexAttribute(uv_index, "in_Uvs");

	// Get uniform bindings for vertex shader
	material->bind();
	projectionMatrixLocation = glGetUniformLocation(shader.getId(), "projectionMatrix");	// Get the location of our projection matrix in the shader
	viewMatrixLocation = glGetUniformLocation(shader.getId(), "viewMatrix");			// Get the location of our view matrix in the shader
	modelMatrixLocation = glGetUniformLocation(shader.getId(), "modelMatrix");		// Get the location of our model matrix in the shader
	noiseLocation = glGetUniformLocation(shader.getId(), "noiseValue");
	textureLocation = glGetUniformLocation(shader.getId(), "myTextureSampler");

	material->unbind();

	//////////////////////////////////////////////////////////////////////////
	// Add Camera
	//////////////////////////////////////////////////////////////////////////
	nap::Entity& camera_entity = core.addEntity("camera");
	cameraComponent = &camera_entity.addComponent<nap::CameraComponent>();
	nap::TransformComponent& camera_transform = camera_entity.addComponent<nap::TransformComponent>();
	camera_transform.translate.setValue({ 0.0f, 0.0f, -4.0f });

	// Set camera
	cameraComponent->fieldOfView.setValue(45.0f);
	cameraComponent->setAspectRatio((float)windowWidth, (float)windowHeight);

	// Create Square Vertex Buffer Object
	createCube(cubeObject, vertex_index, color_index, uv_index);

	// Create triangle Vertex Buffer Object
	createTriangle(triangleObject, vertex_index, color_index, uv_index);

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
				case SDLK_ESCAPE:
					loop = false;
					break;
				case SDLK_f:
				{
					//SDL_SetWindowFullscreen(mainWindow->getWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);
					break;
				}
				case SDLK_PERIOD:
				{
					currentIndex++;
					currentIndex = currentIndex < 3 ? currentIndex : 0;
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
					// Update gl viewport
					updateViewport(event.window.data1, event.window.data2);
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
