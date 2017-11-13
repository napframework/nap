// Local Includes
#include "kalvertorenapp.h"
#include "selectledmeshcomponent.h"

// External Includes
#include <mathutils.h>
#include <basetexture2d.h>
#include <texture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>

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

		// Render window and texture target
		renderWindow = resourceManager->findObject<nap::RenderWindow>("Window0");
		videoTextureTarget = resourceManager->findObject<nap::RenderTarget>("PlaneRenderTarget");
		
		// All of our entities
		kalvertorenEntity = resourceManager->findEntity("KalvertorenEntity");
		modelsEntity = resourceManager->findEntity("ModelsEntity");
		sceneCameraEntity = resourceManager->findEntity("SceneCameraEntity");
		videoCameraEntity = resourceManager->findEntity("VideoCameraEntity");
		defaultInputRouter = resourceManager->findEntity("DefaultInputRouterEntity");
		videoEntity = resourceManager->findEntity("VideoEntity");
		lightEntity = resourceManager->findEntity("LightEntity");

		// Mesh associated with heiligeweg
		heiligeWegMesh = resourceManager->findObject<nap::ArtnetMeshFromFile>("HeiligewegMesh");

		// Materials
		vertexMaterial = resourceManager->findObject<nap::Material>("VertexColorMaterial");
		frameMaterial = resourceManager->findObject<nap::Material>("FrameMaterial");

		// Collect all video resources and play
		videoResource = resourceManager->findObject<nap::Video>("Video1");
		videoResource->play();

		SelectLedMeshComponentInstance& selector = modelsEntity->getComponent<SelectLedMeshComponentInstance>();
		for (auto& mesh : selector.getLedMeshes())
		{
			colorBasedOnBounds(*(mesh->mTriangleMesh));
		}
		mCurrentSelection = selector.getIndex();

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
		entities.push_back(sceneCameraEntity.get());
		inputService->processEvents(*renderWindow, input_router, entities);

		// If the video is not currently playing, start playing it again. This is needed for real time editing; 
		// if the video resource is modified it will not automatically play again (playback is started during init), causing the output to be black
		if (!videoResource->isPlaying())
			videoResource->play();

		// Set the video texture on the material used by the plane
		nap::MaterialInstance& plane_material = videoEntity->getComponent<nap::RenderableMeshComponentInstance>().getMaterialInstance();
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("yTexture").setTexture(videoResource->getYTexture());
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("uTexture").setTexture(videoResource->getUTexture());
		plane_material.getOrCreateUniform<nap::UniformTexture2D>("vTexture").setTexture(videoResource->getVTexture());

		// Copy video texture data over
		if (mVideoBitmap.hasData() && mShowVideo)
			applyVideoTexture();

		// Gui
		SelectLedMeshComponentInstance& selector = modelsEntity->getComponent<SelectLedMeshComponentInstance>();
		ImGui::Begin("Settings");
		if (ImGui::Checkbox("Show Video", &mShowVideo))
		{
			if (!mShowVideo)
			{
				colorBasedOnBounds(*(selector.getLedMeshes()[0]->mTriangleMesh));
			}
		}
		ImGui::SliderFloat("Speed", &(videoResource->mSpeed), 0.0f, 20.0f);
		if (ImGui::SliderInt("Selection", &mCurrentSelection, 0, selector.getCount()-1))
		{
			selector.select(mCurrentSelection);
		}
		ImGui::End();
	}


	void KalvertorenApp::render()
	{
		renderService->destroyGLContextResources(std::vector<nap::ObjectPtr<nap::RenderWindow>>({ renderWindow }));

		// Render offscreen surface(s)
		{
			renderService->getPrimaryWindow().makeCurrent();

			// Video camera
			nap::CameraComponentInstance& video_cam = videoCameraEntity->getComponent<nap::OrthoCameraComponentInstance>();

			// Get plane to render video to
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			components_to_render.push_back(&videoEntity->getComponent<nap::RenderableMeshComponentInstance>());

			// render target
			opengl::TextureRenderTarget2D& render_target = videoTextureTarget->getTarget();
			render_target.setClearColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			renderService->clearRenderTarget(render_target, opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);
			renderService->renderObjects(render_target, video_cam, components_to_render);

			render_target.getColorTexture().getData(mVideoBitmap);
		}


		// Render window 0
		{
			renderWindow->makeActive();

			nap::CameraComponentInstance& sceneCamera = sceneCameraEntity->getComponent<nap::CameraControllerInstance>().getCameraComponent();

			// Render output texture to plane
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			modelsEntity->getComponentsOfType<nap::RenderableComponentInstance>(components_to_render);

			opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
			renderService->clearRenderTarget(backbuffer);
			renderService->renderObjects(backbuffer, sceneCamera, components_to_render);

			// Render our gui
			getCore().getService<IMGuiService>()->render();

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


	void KalvertorenApp::colorBasedOnBounds(ArtnetMeshFromFile& mesh)
	{
		const math::Box& box = mesh.getBoundingBox();

		// Get attributes necessary to color based on bounds
		nap::VertexAttribute<glm::vec4>& color_attr = mesh.getColorAttribute();
		nap::VertexAttribute<glm::vec3>& position_attr = mesh.getPositionAttribute();
		
		int triangle_count = getTriangleCount(mesh.getMeshInstance());

		nap::TriangleDataPointer<glm::vec4> tri_color_data;
		nap::TriangleDataPointer<glm::vec3> tri_posit_data;

		for (int triangle=0; triangle < triangle_count; triangle++)
		{
			// Get current cd values
			getTriangleValues(mesh.getMeshInstance(), triangle, color_attr, tri_color_data);

			// Get current position values
			getTriangleValues(mesh.getMeshInstance(), triangle, position_attr, tri_posit_data);

			// Get avg position value
			glm::vec3 avg_pos(0.0f,0.0f,0.0f);
			for(auto& pos : tri_posit_data)
			{
				avg_pos += *pos;
			}
			avg_pos /= 3;

			float r = math::fit<float>(avg_pos.x, box.getMin().x, box.getMax().x, 0.0f, 1.0f);
			float g = math::fit<float>(avg_pos.y, box.getMin().y, box.getMax().y, 0.0f, 1.0f);
			float b = math::fit<float>(avg_pos.z, box.getMin().z, box.getMax().z, 0.0f, 1.0f);

			// Set vertex colors
			for (auto& col : tri_color_data)
			{
				col->r = pow(r,2.0);
				col->g = pow(g,2.0);
				col->b = pow(b,2.0);
			}
		}

		nap::utility::ErrorState error;
		if (!mesh.getMeshInstance().update(error))
			assert(false);
	}


	void KalvertorenApp::applyVideoTexture()
	{		// Copy pixel data over
		nap::MeshInstance& mesh = heiligeWegMesh->getMeshInstance();

		// UV attribute we use to sample
		nap::VertexAttribute<glm::vec3>& uv_attr = mesh.GetAttribute<glm::vec3>(nap::MeshInstance::VertexAttributeIDs::GetUVName(0));

		// Color attribute we use to sample
		nap::VertexAttribute<glm::vec4>& color_attr = heiligeWegMesh->getColorAttribute();

		// Total amount of triangles
		int tri_count = getTriangleCount(mesh);
		TriangleDataPointer<glm::vec3> tri_uv_data;
		TriangleData<glm::vec4> new_triangle_color;
		for (int i = 0; i < tri_count; i++)
		{
			// Get uv coordinates for that triangle
			getTriangleValues<glm::vec3>(mesh, i, uv_attr, tri_uv_data);

			// Average uv values
			glm::vec2 uv_avg{ 0.0,0.0 };
			for (const auto& uv_vertex : tri_uv_data)
			{
				uv_avg.x += uv_vertex->x;
				uv_avg.y += uv_vertex->y;
			}
			uv_avg.x = uv_avg.x / 3.0f;
			uv_avg.y = uv_avg.y / 3.0f;

			// Convert to pixel coordinates
			int x_pixel = static_cast<float>(mVideoBitmap.getWidth() - 1) * uv_avg.x;
			int y_pixel = static_cast<float>(mVideoBitmap.getWidth() - 1) * uv_avg.y;

			// retrieve pixel value
			uint8* pixel_pointer = mVideoBitmap.getPixel<uint8>(x_pixel, y_pixel);

			// iterate over every vertex in the triangle and set the color
			for (auto& vert_color : new_triangle_color)
			{
				vert_color.r = static_cast<float>(pixel_pointer[0] / 255.0f);
				vert_color.g = static_cast<float>(pixel_pointer[1] / 255.0f);
				vert_color.b = static_cast<float>(pixel_pointer[2] / 255.0f);
				vert_color.a = 1.0f;
			}

			setTriangleValues<glm::vec4>(mesh, i, color_attr, new_triangle_color);
		}

		nap::utility::ErrorState error;
		if (!mesh.update(error))
		{
			assert(false);
		}
	}

}