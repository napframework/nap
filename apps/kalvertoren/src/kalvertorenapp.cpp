// Local Includes
#include "kalvertorenapp.h"
#include "selectledmeshcomponent.h"
#include "selectcolormethodcomponent.h"
#include "applytracercolorcomponent.h"
#include "applybbcolorcomponent.h"
#include "applycompositioncomponent.h"
#include "rendercompositioncomponent.h"
#include "rendervideocomponent.h"

// External Includes
#include <mathutils.h>
#include <texture2d.h>
#include <rendertexture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>
#include <utility/stringutils.h>
#include <scene.h>
#include <planemesh.h>
#include <ctime>
#include <chrono>
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KalvertorenApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	bool KalvertorenApp::init(utility::ErrorState& error)
	{
		// Create services
		renderService = getCore().getService<nap::RenderService>();
		inputService =  getCore().getService<nap::InputService>();
		sceneService =  getCore().getService<nap::SceneService>();

		// Get resource manager and load data
		resourceManager = getCore().getResourceManager();
		if (!resourceManager->loadFile("kalvertoren.json", error))
			return false;    

		// Render window and texture target
		renderWindow = resourceManager->findObject<nap::RenderWindow>("Window0");
		
		// Callback when window event is received
		renderWindow->mWindowEvent.connect(std::bind(&KalvertorenApp::handleWindowEvent, this, std::placeholders::_1));

		// All of our entities
		rtti::ObjectPtr<Scene> scene = resourceManager->findObject<Scene>("Scene");

		// Entities
		compositionEntity = scene->findEntity("CompositionEntity");
		renderCompositionEntity = scene->findEntity("RenderCompositionEntity");
		renderVideoEntity = scene->findEntity("RenderVideoEntity");
		displayEntity = scene->findEntity("DisplayEntity");
		sceneCameraEntity = scene->findEntity("SceneCameraEntity");
		compositionCameraEntity = scene->findEntity("CompositionCameraEntity");
		defaultInputRouter = scene->findEntity("DefaultInputRouterEntity");
		lightEntity = scene->findEntity("LightEntity");

		// Materials
		vertexMaterial = resourceManager->findObject<nap::Material>("VertexColorMaterial");
		frameMaterial = resourceManager->findObject<nap::Material>("FrameMaterial");

		// Set render states
		nap::RenderState& render_state = renderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;

		// Create gui
		mGui = std::make_unique<KalvertorenGui>(*this);
		mGui->init();

		// Start logging to directory
		nap::Logger::logToDirectory(utility::getExecutableDir());

		return true;
	}

	void KalvertorenApp::update(double deltaTime)
	{
		nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(sceneCameraEntity.get());
		inputService->processEvents(*renderWindow, input_router, entities);

		// Update our gui
		mGui->update(deltaTime);
	}


	void KalvertorenApp::render()
	{
		renderService->destroyGLContextResources(std::vector<rtti::ObjectPtr<nap::RenderWindow>>({ renderWindow }));

		// Render offscreen surface(s)
		{
			renderService->getPrimaryWindow().makeCurrent();

			// Render video
			RenderVideoComponentInstance& video_render = renderVideoEntity->getComponent<RenderVideoComponentInstance>();
			video_render.render();

			// Render compositions
			RenderCompositionComponentInstance& comp_render = renderCompositionEntity->getComponent<RenderCompositionComponentInstance>();
			comp_render.render();
		}


		// Render window 0
		{
			renderWindow->makeActive();

			// Clear backbuffer of window
			opengl::RenderTarget& backbuffer = renderWindow->getBackbuffer();
			renderService->clearRenderTarget(backbuffer);

			// Get camera
			nap::CameraComponentInstance& sceneCamera = sceneCameraEntity->getComponent<nap::CameraControllerInstance>().getCameraComponent();

			// Render meshes
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			displayEntity->getComponentsOfType<nap::RenderableComponentInstance>(components_to_render);
			renderService->renderObjects(backbuffer, sceneCamera, components_to_render);

			// Render our gui
			mGui->draw();

			// Swap buffers
			renderWindow->swap();
		}
	}


	void KalvertorenApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
	}


	int KalvertorenApp::shutdown()
	{
		mGui.reset();
		return 0;
	}


	void KalvertorenApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		renderService->addEvent(std::move(windowEvent));
	}


	void KalvertorenApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit();
				return;
			}

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				static bool fullscreen = true;
				resourceManager->findObject<nap::RenderWindow>("Window0")->getWindow()->setFullScreen(fullscreen);
				fullscreen = !fullscreen;
				return;
			}
		}

		// Add event to input service for further processing
		inputService->addEvent(std::move(inputEvent));
	}
}