// Local Includes
#include "kalvertorenapp.h"

// External Includes
#include <mathutils.h>
#include <basetexture2d.h>
#include <texture2d.h>

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
		videoService =  getCore().getService<nap::VideoService>();

		// Get resource manager and load data
		resourceManager = getCore().getResourceManager();
		if (!resourceManager->loadFile("data/kalvertoren/kalvertoren.json", error))
			return false;

		renderWindow = resourceManager->findObject<nap::RenderWindow>("Window0");
		textureRenderTarget = resourceManager->findObject<nap::RenderTarget>("PlaneRenderTarget");
		kalvertorenEntity = resourceManager->findEntity("KalvertorenEntity");
		cameraEntity = resourceManager->findEntity("CameraEntity");
		defaultInputRouter = resourceManager->findEntity("DefaultInputRouterEntity");
		videoEntity = resourceManager->findEntity("VideoEntity");
		lightEntity = resourceManager->findEntity("LightEntity");
		vertexMaterial = resourceManager->findObject<nap::Material>("VertexColorMaterial");
		frameMaterial = resourceManager->findObject<nap::Material>("FrameMaterial");

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

	void KalvertorenApp::update(double deltaTime)
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


	void KalvertorenApp::render()
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


	void KalvertorenApp::shutdown()
	{

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
				quit(0);
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


	void KalvertorenApp::updateCameraLocation()
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
}